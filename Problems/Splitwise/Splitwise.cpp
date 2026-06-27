#include <bits/stdc++.h>
using namespace std ;

class User {
    private:
    int id ;
    string name ;
    string email ;
    public:
    User(){}
    User ( int id ,string name , string email ) {
        this -> id = id  ;
        this -> name = name ; 
        this -> email = email ;
    }
    int getId() { return id; }
    string getName() { return name; }
    string getEmail() { return email; }
};

class Group ;

// user Repo will only have direct access to the User pointer
// for any other entity we will expect a id and use that to get information 
// to reduce tight coupling
class UserRepository {
    private:
    unordered_map < int , User* > userDb;
    unordered_map < int , set < int >*> userGroups ;
    /// map < pair < int , int > , unordered_map < int , int >> userTransaction ;
    public:
    UserRepository(){}

    void addUser (User* user) {
        userDb[user->getId()] = user ;
        userGroups[user->getId()] = new set<int>() ;
        cout << "[User Repository] Added user: " << user->getName() << " (ID: " << user->getId() << ")" << endl;
    }
    
    bool userExists (int id) {
        if (userDb.count(id) ) {
            return true ;
        }
        return false ;
    }

    User* getUser (int id) {
        return userDb[id] ;
    }

    void addUserToGroup(int id , int groupId) {
        if (userGroups.count(id)) {
            userGroups[id]->insert(groupId) ;
        }
    }

    set<int>* getUserGroupsIds (int id) {
        return userGroups[id] ;
    }

    // void addUserTransaction ( int group)
};

class GroupService ;
class GroupRepository ;
// we will see later if we need to create a UserService 
class UserService {
    private:
    UserRepository *repo ;
    GroupRepository *groupRepo ;
    GroupService * groupService;
    int id = 0 ; 
    public:
    UserService (UserRepository *repo , GroupRepository *groupRepo , GroupService *groupService) {
        id = 0 ; 
        this->repo = repo ;
        this->groupRepo = groupRepo ; 
        this->groupService = groupService ;
    }

    User *createUser () {
        User *u = new User (id , to_string(id) + "name" , to_string(id) + "name@gmail.com") ;
        repo->addUser(u) ;
        cout << "[User Service] Created user: " << u->getName() << " (ID: " << u->getId() << ")" << endl;
        id++ ; 
        return u ;
    }

};

class Group {
    private:
    int id ; 
    string name ;
    public:
    Group(){}
    Group(int id , string name) {
        this->id = id ;
        this->name = name ;
    }
    int getId() { return id; }
    string getName() { return name; }
};

class GroupRepository {
    friend class ResolveTransactionService; // Fixed encapsulation breakdown

    private:
    unordered_map < int , Group* > groups ;
    unordered_map < int , set < int >*> userList ;
    public:
    unordered_map < int , unordered_map < int , set <int> > > transactions ;
    unordered_map < int, map < pair < int , int > , float > > amount ; // Fixed cross-group debt pollution
    GroupRepository(){}

    void addGroup(int id , Group* g) {
        groups[id] = g ;
        userList[id] = new set<int>() ;
        cout << "[Group Repository] Added group: " << g->getName() << " (ID: " << g->getId() << ")" << endl;
    }

    Group* getGroup (int id) {
        return groups[id] ;
    }

    set < int > * gerUserList (int groupId ) {
        return userList[groupId] ;
    }

    void addTransaction (int groupId , int userId1 , int userId2 , float amt ) {
        unordered_map<int , set <int>>& t =  transactions[groupId] ;
        t[userId1].insert(userId2) ;
        amount[groupId][{userId1 , userId2}] += amt; // Scoped to groupId
    }
};

class ISplitStrategy {
    protected:
    UserRepository *userRepo ;
    GroupRepository *groupRepo ;
    public:
    virtual ~ISplitStrategy() = default ;
    virtual void split (int groupId , int userPaid , float value , vector < int > notInclude ) = 0 ;
};

class EqualSplit : public ISplitStrategy {
    public:
    EqualSplit(UserRepository *userRepo ,
    GroupRepository *groupRepo ) {
        this -> userRepo = userRepo ;
        this -> groupRepo = groupRepo ;
    }

    void split (int groupId , int userPaid , float value , vector < int > notInclude ) {
        set < int >* userList = groupRepo->gerUserList(groupId);
        float valuePerUser = value / (userList->size() - notInclude.size()) ;
        for ( auto x : *userList ) {
            if (std::find(notInclude.begin(), notInclude.end(), x) != notInclude.end()) {
                continue; // Skip this user
            }
            groupRepo->addTransaction(groupId , x , userPaid , valuePerUser);
        }
    }
};

class PercentageSplit : public ISplitStrategy {
    private:
    unordered_map < int , float >breakdown ;
    public:
    PercentageSplit(UserRepository *userRepo ,
    GroupRepository *groupRepo , unordered_map < int , float > breakdown ) {
        this -> userRepo = userRepo ;
        this -> groupRepo = groupRepo ;
        this-> breakdown = breakdown ;
    }
    void split (int groupId , int userPaid , float value , vector < int > notInclude ) {
        set < int >* userList = groupRepo->gerUserList(groupId);
        
        for ( auto x : *userList ) {
            float valuePerUser = value * breakdown[x] / 100 ;
            if (std::find(notInclude.begin(), notInclude.end(), x) != notInclude.end()) {
                continue; // Skip this user
            }
            groupRepo->addTransaction(groupId , x , userPaid , valuePerUser);
        }
    }
};

class GroupService {
    private:
    GroupRepository *groupRepo ;
    UserRepository *userRepo ;
    int id ;
    public:
    GroupService (UserRepository *userRepo , GroupRepository *groupRepo ) {
        this->userRepo = userRepo ; 
        this->groupRepo = groupRepo ;
        id = 0 ; 
    }
    Group* createGroup () {
        Group *g = new Group (id , to_string(id) + "GroupName") ;
        groupRepo->addGroup(id , g) ;
        cout << "[Group Service] Created group: " << g->getName() << " (ID: " << g->getId() << ")" << endl;
        id++ ;
        return g ;
    }

    void addUser (int id , int groupId) {
        userRepo->addUserToGroup(id , groupId) ;
        set<int>* list = groupRepo->gerUserList(groupId) ;
        if (list) {
            list->insert(id) ;
        }
        cout << "[Group Service] Added user ID " << id << " to group ID " << groupId << endl;
    }

    void deletedUser ( int id , int groupId ) {
        cout << "[Group Service] : User " << userRepo->getUser(id)->getName() << " left the Group " << groupRepo->getGroup(groupId)->getName() << endl ;
        set < int >*list = userRepo->getUserGroupsIds(id) ;
        if (list) {
            list->erase(groupId) ;
        }

        list = groupRepo->gerUserList(groupId) ;
        if (list) {
            list->erase(id) ;
        }
    }

    void simulate (int groupId) {
        set<int>* members = groupRepo->gerUserList(groupId);
        if (!members || members->size() < 2) return;

        vector<int> users(members->begin(), members->end());
        int payer = users[rand() % users.size()];
        float amount = (rand() % 901) + 100; // Random amount between 100 and 1000
        int strategyType = rand() % 2;

        if (strategyType == 0) {
            cout << "[Simulation] Running Equal Split. Payer: " << payer << ", Amount: " << amount << endl;
            EqualSplit solver(userRepo, groupRepo);
            solver.split(groupId, payer, amount, {});
        } else {
            cout << "[Simulation] Running Percentage Split. Payer: " << payer << ", Amount: " << amount << endl;
            unordered_map<int, float> breakdown;
            float total = 0;
            vector<float> weights(users.size());
            for (size_t i = 0; i < users.size(); ++i) {
                weights[i] = (rand() % 100) + 1;
                total += weights[i];
            }
            for (size_t i = 0; i < users.size(); ++i) {
                breakdown[users[i]] = (weights[i] / total) * 100.0f;
            }
            PercentageSplit solver(userRepo, groupRepo, breakdown);
            solver.split(groupId, payer, amount, {});
        }
    }
};

unordered_map<int, vector<pair<int, float>>> adjList;
int mask ;
int n ;
unordered_map < int , int > maskScore ; 

class ResolveTransactionService {
    private:
    UserRepository* userRepo ;
    GroupRepository* groupRepo ; 
    public:
    ResolveTransactionService(UserRepository *userRepo , GroupRepository *groupRepo) {
        this -> userRepo = userRepo ;
        this -> groupRepo = groupRepo ;
    }

    void resolve(int groupId) {
        adjList.clear();
        mask = 0 ;
        set<int>* members = groupRepo->gerUserList(groupId);
        
        if (!members || groupRepo->amount.find(groupId) == groupRepo->amount.end()) {
            return;
        }

        auto& groupAmounts = groupRepo->amount[groupId];
        vector<int> users(members->begin(), members->end());
        n = users.size();

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                int u1 = users[i];
                int u2 = users[j];

                float owed_u1_to_u2 = 0.0f;
                float owed_u2_to_u1 = 0.0f;

                if (groupAmounts.count({u1, u2})) {
                    owed_u1_to_u2 = groupAmounts[{u1, u2}];
                }
                if (groupAmounts.count({u2, u1})) {
                    owed_u2_to_u1 = groupAmounts[{u2, u1}];
                }

                float netDebt = owed_u1_to_u2 - owed_u2_to_u1;
                if (netDebt > 0) {
                    adjList[u1].push_back({u2, netDebt});
                }
            }
        }

        // Calculate Net Balances for each user in the group
        vector<float> balances(n, 0.0f);
        unordered_map<int, int> userToIdx;
        vector<int> idxToUser(n);
        
        for (int i = 0; i < n; ++i) {
            userToIdx[users[i]] = i;
            idxToUser[i] = users[i];
        }

        for (int i = 0; i < n; ++i) {
            int u1 = users[i];
            for (auto& edge : adjList[u1]) {
                int u2 = edge.first;
                float amt = edge.second;
                balances[userToIdx[u1]] -= amt; // u1 owes money
                balances[userToIdx[u2]] += amt; // u2 receives money
            }
        }

        // Bitmask DP to find maximum number of zero-sum components
        int totalMasks = 1 << n;
        vector<float> sumMesh(totalMasks, 0.0f);
        vector<int> dp(totalMasks, 0);
        vector<int> split(totalMasks, -1);

        for (int i = 0; i < totalMasks; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i & (1 << j)) {
                    sumMesh[i] += balances[j];
                }
            }
        }

        for (int m = 1; m < totalMasks; ++m) {
            if (abs(sumMesh[m]) < 1e-3) {
                dp[m] = 1;
            }
            for (int submask = (m - 1) & m; submask > 0; submask = (submask - 1) & m) {
                if (dp[submask] + dp[m ^ submask] > dp[m]) {
                    dp[m] = dp[submask] + dp[m ^ submask];
                    split[m] = submask;
                }
            }
        }

        // Reconstruct independent zero-sum groups
        vector<int> components;
        queue<int> q;
        q.push(totalMasks - 1);

        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            if (split[curr] != -1) {
                q.push(split[curr]);
                q.push(curr ^ split[curr]);
            } else {
                components.push_back(curr);
            }
        }

        // Wipe old transaction databases for this group to override with optimized values
        groupRepo->amount[groupId].clear();
        groupRepo->transactions[groupId].clear();

        // Greedy matching calculation inside each isolated component
        for (int compMask : components) {
            vector<int> debtors, creditors;
            vector<float> localBalances = balances;

            for (int j = 0; j < n; ++j) {
                if (compMask & (1 << j)) {
                    if (localBalances[j] < -1e-3) debtors.push_back(j);
                    else if (localBalances[j] > 1e-3) creditors.push_back(j);
                }
            }

            size_t dIdx = 0, cIdx = 0;
            while (dIdx < debtors.size() && cIdx < creditors.size()) {
                int d = debtors[dIdx];
                int c = creditors[cIdx];

                float settleAmt = min(-localBalances[d], localBalances[c]);
                localBalances[d] += settleAmt;
                localBalances[c] -= settleAmt;

                int actualDebtor = idxToUser[d];
                int actualCreditor = idxToUser[c];

                groupRepo->addTransaction(groupId, actualDebtor, actualCreditor, settleAmt);
                cout << "[Resolve Service] User ID " << actualDebtor << " pays " << settleAmt << " to User ID " << actualCreditor << endl;

                if (abs(localBalances[d]) < 1e-3) dIdx++;
                if (abs(localBalances[c]) < 1e-3) cIdx++;
            }
        }
    }
};

class TransactionService {
    private:
    UserRepository* userRepo ;
    GroupRepository* groupRepo ; 
    public:

    void resolve() {}
};

int main ()  {
    srand(time(0));
    // registering all the repositories 
    UserRepository *userRepo = new UserRepository() ;
    GroupRepository *groupRepo = new GroupRepository() ;

    // services 
    GroupService *groupService = new GroupService(userRepo , groupRepo);
    UserService *userService = new UserService(userRepo , groupRepo , groupService) ;

    // creating all users 
    User *user1 = userService->createUser() ;
    User *user2 = userService->createUser() ;
    User *user3 = userService->createUser() ;
    User *user4 = userService->createUser() ;
    User *user5 = userService->createUser() ;

    // creating groups
    Group *group = groupService->createGroup() ;

    groupService->addUser(user1->getId(), group->getId());
    groupService->addUser(user2->getId(), group->getId());
    groupService->addUser(user3->getId(), group->getId());
    groupService->addUser(user4->getId(), group->getId());
    groupService->addUser(user5->getId(), group->getId());

    // Simulating random transaction activities
    groupService->simulate(group->getId());
  //  groupService->simulate(group->getId());

    // Resolve and minimize cash flows via Bitmask DP
    ResolveTransactionService resolver(userRepo, groupRepo);
    resolver.resolve(group->getId());

    // Clean up memory
    delete userService;
    delete groupService;
    delete groupRepo;
    delete userRepo;

    return 0;
}