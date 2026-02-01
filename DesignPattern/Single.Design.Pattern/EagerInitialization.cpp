
#include <bits/stdc++.h>
using namespace std ; 
/*
    allows singleton activation 
    and also allows to be thread safe by defualt
    since we crate the object even before the main function hits
    ***can be left unused , but usually takes up tiny memory

    *** we achieve this using the static intitialization

    all the other fucntionality remains the same
*/
class Logger {
    private:
    static Logger* instance ; 
    Logger () {
        cout << "constructor called " << endl ; 
    }
    ~Logger () {
        cout << "Destructor called " << endl ;
    }

    // prevent copy and assignment operator 
    Logger ( const Logger& ) ;
    Logger& operator= ( const Logger& ) ;

    public:
    static Logger* getInstance() {
        return instance ; 
    }
    static void destroyInstance () {
        if ( instance != nullptr ) {
            delete instance ; // *** memory is free , but the instance stll holds the pointer to the old address
            instance = nullptr ; 
        }
    }
    
    void log ( const char* msg ) {
        cout << "[LOG] " << msg << endl ; 
    }
};

/*
    the cool stuff i sher e
    in lazy init , we set the this to nullptr 
    in eager init , we set this to new Logger()
    this runs before the main starts as well
*/

Logger * Logger::instance = new Logger() ;

int main() {

    cout << "Main Function Intitialized , notice how the constructor is called already" << endl ;
    Logger::getInstance()->log("Hello World from eager") ;

    Logger::destroyInstance() ;
}