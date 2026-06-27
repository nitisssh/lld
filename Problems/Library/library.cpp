#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <ctime>

using namespace std;

// ==========================================
// 1. DOMAIN MODELS (Thread-Safe & Encapsulated)
// ==========================================

class Book {
private:
    int id;
    string name;
    string author;
    string ISBN;
    int publicationYear;
    bool isAvailable;
    mutable mutex bookMtx; // Protects individual book state transitions

public:
    Book(int id, string name, string author, string ISBN, int publicationYear)
        : id(id), name(name), author(author), ISBN(ISBN), publicationYear(publicationYear), isAvailable(true) {}

    int getId() const { return id; }
    
    // Thread-safe status check and flip (Atomic transition)
    bool tryBorrow() {
        lock_guard<mutex> lock(bookMtx);
        if (isAvailable) {
            isAvailable = false;
            return true;
        }
        return false;
    }

    void markAvailable() {
        lock_guard<mutex> lock(bookMtx);
        isAvailable = true;
    }

    bool checkAvailability() const {
        lock_guard<mutex> lock(bookMtx);
        return isAvailable;
    }
};

class Member {
private:
    int id;
    string name;
    string contactInfo;

public:
    Member(int id, string name, string contactInfo)
        : id(id), name(name), contactInfo(contactInfo) {}

    int getId() const { return id; }
};

class Borrow {
private:
    int id;
    int memberId;
    int bookId;
    time_t borrowDate;
    time_t returnDate;
    bool returned;
    mutable mutex borrowMtx;

public:
    Borrow(int id, int memberId, int bookId, time_t borrowDate)
        : id(id), memberId(memberId), bookId(bookId), borrowDate(borrowDate), returnDate(0), returned(false) {}

    int getId() const { return id; }
    int getMemberId() const { return memberId; }
    int getBookId() const { return bookId; }
    
    bool isReturned() {
        lock_guard<mutex> lock(borrowMtx);
        return returned;
    }

    void markReturned() {
        lock_guard<mutex> lock(borrowMtx);
        if (!returned) {
            returned = true;
            returnDate = time(nullptr);
        }
    }
};

// ==========================================
// 2. REPOSITORIES (Using Smart Pointers & Clear Mutexes)
// ==========================================

class BookRepository {
private:
    unordered_map<int, shared_ptr<Book>> books;
    mutex repoMtx;

public:
    void addBook(shared_ptr<Book> book) {
        lock_guard<mutex> lock(repoMtx);
        books[book->getId()] = book;
    }

    shared_ptr<Book> findById(int id) {
        lock_guard<mutex> lock(repoMtx);
        auto it = books.find(id);
        if (it != books.end()) return it->second;
        return nullptr;
    }
};

class IMemberRepository {
public:
    virtual ~IMemberRepository() = default;
    virtual void insert(shared_ptr<Member> member) = 0;
    virtual shared_ptr<Member> findById(int id) = 0;
};

class MongoMemberRepository : public IMemberRepository {
private:
    unordered_map<int, shared_ptr<Member>> members;
    mutex repoMtx;

public:
    void insert(shared_ptr<Member> member) override {
        lock_guard<mutex> lock(repoMtx);
        members[member->getId()] = member;
    }

    shared_ptr<Member> findById(int id) override {
        lock_guard<mutex> lock(repoMtx);
        auto it = members.find(id);
        if (it != members.end()) return it->second;
        return nullptr;
    }
};

class IBorrowRepository {
public:
    virtual ~IBorrowRepository() = default;
    virtual void insert(shared_ptr<Borrow> borrow) = 0;
    virtual void update(shared_ptr<Borrow> borrow) = 0;
    virtual shared_ptr<Borrow> findById(int id) = 0;
    virtual int getActiveBorrowCount(int memberId) = 0;
};

class BorrowRepository : public IBorrowRepository {
private:
    unordered_map<int, shared_ptr<Borrow>> borrows;
    unordered_map<int, int> activeBorrowCounts; // Tracks active unreturned books per member
    mutex repoMtx;

public:
    void insert(shared_ptr<Borrow> borrow) override {
        lock_guard<mutex> lock(repoMtx);
        borrows[borrow->getId()] = borrow;
        if (!borrow->isReturned()) {
            activeBorrowCounts[borrow->getMemberId()]++;
        }
    }

    void update(shared_ptr<Borrow> borrow) override {
        lock_guard<mutex> lock(repoMtx);
        // Counting rules are updated here inside the repository lock safely
        auto it = borrows.find(borrow->getId());
        if (it != borrows.end()) {
            // If it was active and now marked returned, reduce active count
            if (borrow->isReturned()) {
                activeBorrowCounts[borrow->getMemberId()] = max(0, activeBorrowCounts[borrow->getMemberId()] - 1);
            }
        }
    }

    shared_ptr<Borrow> findById(int id) override {
        lock_guard<mutex> lock(repoMtx);
        auto it = borrows.find(id);
        if (it != borrows.end()) return it->second;
        return nullptr;
    }

    int getActiveBorrowCount(int memberId) override {
        lock_guard<mutex> lock(repoMtx);
        return activeBorrowCounts[memberId];
    }
};

// ==========================================
// 3. RULES STRATEGY (Evaluated Under Transaction Locks)
// ==========================================

class IRules {
public:
    virtual ~IRules() = default;
    // Must accept current repository to accurately check current transaction state
    virtual bool canBorrow(int memberId, int bookId, IBorrowRepository* borrowRepo) = 0;
};

class MaxBooksRule : public IRules {
private:
    int maxBooks;
public:
    MaxBooksRule(int maxBooks) : maxBooks(maxBooks) {}
    
    bool canBorrow(int memberId, int bookId, IBorrowRepository* borrowRepo) override {
        return borrowRepo->getActiveBorrowCount(memberId) < maxBooks;
    }
};

// ==========================================
// 4. CORE SERVICES (Thread-Safe Transaction Coordinators)
// ==========================================

class BorrowService {
private:
    shared_ptr<IBorrowRepository> borrowRepository;
    shared_ptr<BookRepository> bookRepository;
    vector<shared_ptr<IRules>> rules;
    int nextBorrowId = 1;
    mutex serviceMtx; // Atomic transactional block lock to avoid TOCTOU bugs

public:
    BorrowService(shared_ptr<IBorrowRepository> borrowRepo, shared_ptr<BookRepository> bookRepo)
        : borrowRepository(borrowRepo), bookRepository(bookRepo) {}

    void addRule(shared_ptr<IRules> rule) {
        lock_guard<mutex> lock(serviceMtx);
        rules.push_back(rule);
    }

    bool borrowBook(int memberId, int bookId) {
        // Enforce sequence locking to completely avoid Deadlocks. 
        // This single lock makes rule evaluation + book allocation atomic.
        lock_guard<mutex> lock(serviceMtx);

        shared_ptr<Book> book = bookRepository->findById(bookId);
        if (!book) {
            cout << "Operation failed: Book ID " << bookId << " does not exist.\n";
            return false;
        }

        // 1. Evaluate business rules safely inside the transaction block
        for (const auto& rule : rules) {
            if (!rule->canBorrow(memberId, bookId, borrowRepository.get())) {
                cout << "Borrowing denied: Member " << memberId << " violated library rules.\n";
                return false;
            }
        }

        // 2. Try to secure the book atomically
        if (book->tryBorrow()) {
            auto borrowTx = std::make_shared<Borrow>(nextBorrowId++, memberId, bookId, time(nullptr));
            borrowRepository->insert(borrowTx);
            cout << "Success: Member " << memberId << " successfully borrowed book " << bookId << ".\n";
            return true;
        }

        cout << "Borrowing denied: Book " << bookId << " is already loaned out.\n";
        return false;
    }

    void returnBook(int borrowId) {
        lock_guard<mutex> lock(serviceMtx);
        
        shared_ptr<Borrow> borrow = borrowRepository->findById(borrowId);
        if (borrow && !borrow->isReturned()) {
            borrow->markReturned();
            borrowRepository->update(borrow);

            shared_ptr<Book> book = bookRepository->findById(borrow->getBookId());
            if (book) {
                book->markAvailable();
            }
            cout << "Success: Borrow transaction " << borrowId << " closed. Book returned.\n";
        } else {
            cout << "Operation failed: Invalid transaction ID or book already returned.\n";
        }
    }
};

class Library {
private:
    shared_ptr<BookRepository> bookRepository;
    shared_ptr<IMemberRepository> memberRepository;
    shared_ptr<BorrowService> borrowService;

public:
    Library(shared_ptr<BookRepository> bRepo, shared_ptr<IMemberRepository> mRepo, shared_ptr<BorrowService> bService)
        : bookRepository(bRepo), memberRepository(mRepo), borrowService(bService) {}

    void borrowBook(int memberId, int bookId) {
        borrowService->borrowBook(memberId, bookId);
    }

    void returnBook(int borrowId) {
        borrowService->returnBook(borrowId);
    }
};

int main() {
    // Memory management handled cleanly using smart pointers. Zero memory leaks.
    auto bookRepo = make_shared<BookRepository>();
    auto memberRepo = make_shared<MongoMemberRepository>();
    auto borrowRepo = make_shared<BorrowRepository>();

    auto borrowService = make_shared<BorrowService>(borrowRepo, bookRepo);
    Library library(bookRepo, memberRepo, borrowService);

    // Attach business rules
    borrowService->addRule(make_shared<MaxBooksRule>(2)); // Limit set to 2 books max for testing

    // Catalog setup
    auto b1 = make_shared<Book>(101, "Modern C++ Design", "Andrei Alexandrescu", "9780201704310", 2001);
    auto b2 = make_shared<Book>(102, "Effective Modern C++", "Scott Meyers", "9781491903995", 2014);
    auto b3 = make_shared<Book>(103, "The C++ Programming Language", "Bjarne Stroustrup", "9780321563842", 2013);
    bookRepo->addBook(b1);
    bookRepo->addBook(b2);
    bookRepo->addBook(b3);

    auto m1 = make_shared<Member>(1, "Alice Smith", "alice@example.com");
    memberRepo->insert(m1);

    // Test executions
    library.borrowBook(1, 101); // Allowed
    library.borrowBook(1, 102); // Allowed
    library.borrowBook(1, 103); // Denied! Exceeds limit rule of 2

    library.returnBook(1);      // Return first book
    library.borrowBook(1, 103); // Allowed now! Slot freed up.

    return 0;
}