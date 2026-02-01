/*
    gold standard for the singleton design pattern
    achieves what billpugh appraoch does in java
    thread safety + lazy intialization , without using any lock 

    lives on the stack , no new keyword used 
*/

#include <bits/stdc++.h>
using namespace std ; 

class Logger {
    private:
    Logger() {
        cout << "Construtor called " << endl ;
    }
    ~Logger() {
        cout << "Distructor called " << endl ; 
    }

    Logger ( const Logger&) = delete ; // explicetly tell the compiler not to generate the fucntion and forbid from calling it

    public:
    static Logger& getInstance() {
        static Logger instance ; 
        return instance ; 
    }
    void log (const char* msg) {
        cout << "[LOG] " << msg << endl ; 
    }
};

int main() {
    Logger::getInstance().log("hello") ;
}