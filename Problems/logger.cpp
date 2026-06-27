#include <bits/stdc++.h>
#include <format>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
using namespace std ;

// so basically has-a and is-a to make sure it is the same type
// yet exploit the other objects fucntion and wrap the behaviour above it

class ILogger {
    public:
    virtual ~ILogger () = default ;
    virtual string log ( string data ) = 0 ;  
};

class NormalLogger : public ILogger {
    public:
    NormalLogger(){}
    string log (string data ) {
        return " " + data + " " ;
    }
};

class ItalicLogger : public ILogger {
    public:
    ItalicLogger(){}
    string log (string data ) {
        return " \033[3m" + data + "\033[0m " ;
    }
};

// i want to add some feature on this dumbass logger 
// is a logger because i want to exploit this stricture
// will allow me to use the run-time polymorphism
class IDecorator : public ILogger {
    protected:
    ILogger *logger ;
    public :
    virtual ~IDecorator () = default ;
    virtual string log (string data ) = 0 ;
};

class TimeStampDecorator : public IDecorator {
    private:
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t current_time = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&current_time), "%Y-%m-%d %H:%M:%S") << "] ";
        return ss.str();
    }
    public:
    TimeStampDecorator (ILogger *logger ) {
        this -> logger = logger ;
    }

    string log ( string data ) {
        return getCurrentTime() + logger->log(data);
    }
};

class LevelDecorator : public IDecorator {
    public:
    LevelDecorator (ILogger *logger ) {
        this -> logger = logger ;
    }
    
    string log ( string data ) {
        return " [INFO] " + logger->log(data) ;
    }
};

int main () {
    ILogger *loggerWithDecorators = new TimeStampDecorator (new ItalicLogger()) ;
    cout << loggerWithDecorators -> log("Hello from Console!") << endl ;
    loggerWithDecorators = new LevelDecorator (new TimeStampDecorator (new ItalicLogger()) );
    cout << loggerWithDecorators -> log("Hello from Console!") << endl ;
    loggerWithDecorators = new TimeStampDecorator (new LevelDecorator (new NormalLogger()) );
    cout << loggerWithDecorators -> log("Hello from Console!") << endl ;
}