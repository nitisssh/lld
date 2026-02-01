#include <bits/stdc++.h>
#include <mutex>
using namespace std ;

class Logger {
    private:
    static Logger *instance ; 
    static std::mutex mtx ;
    Logger () {
        cout << "Constructor called " << endl ; 
    }
    ~Logger () {
        cout << "Destructor called " << endl ;
    }
    Logger ( const Logger& ); // dont allow the copy 
    Logger& operator=( const Logger&) ;// dont allow assignmnet
    
    public:

    static Logger* getInstance () {
        if ( instance == NULL ) { // preventive check , as lock is expensive and blocking
            mtx.lock() ;
            if ( instance == nullptr ) {
                instance = new Logger() ;
            }
            mtx.unlock() ;
        }
        return instance ; 
    }
    void log(const char* msg) {
        cout << "[LOG] " << msg << endl;
    }
};

// static member intialization
Logger * Logger::instance = nullptr ;
std::mutex Logger::mtx;  

int main() {
    Logger::getInstance()->log("thread safe hellow world");
}