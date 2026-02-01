/*
 lazy initializaton ( only when called for the first time )
 no thread safety

 singleton is used for non sharable and the objects that can exist only one instance
 and shall be shared and dealt with 

 for eg -> loggers , caches , connection pool 
*/

#include <bits/stdc++.h>
using namespace std ;


class Logger {
    private:
    /*
    why static ?
    static members belongs to the class itself , not to any specific object.
    There is only one variable 'instance' shared by the entire program 
    or list of programs even if included in any other files 

    why a pointer ?
    Using a pointer allows us to initialise it to NULL , this lets us check if the object exists yet  
    allowing us the lazy loading capabilities
    */
    static Logger* instance ; 

    /*
        Why private constructor ?
        we need to prevent external code like main() or any object to do something 
        like new Logger()  , which will create a new intsace 
        so we make it private , it will give compilation error if we tried to crate one object 
    */
    Logger() {
        cout << "Logger Constructed" << endl ;
    }
    /*
        Why private destructor ?   
        Prevents users from calling the delete on the pointer manually 
        only class should be able to do it 
    */
    ~Logger () {
        cout << "Logger Destructed" << endl ; 
    }
    /*
        Why private copy constructor ?
        c++ provides with the default copy ctr but it does ***shallow copy
        we dont wnat someone to use the copy method to create the new object 
        ** const is not necessary here , const is just there to make sure it doesn't alter the original 
           object which we are trying to copy
    */
   Logger ( const Logger& ) ; // not adding any behaviour

   /*
        Why private assignmnet operator ?
        c++ provides with the default assignment operator
        so we need to dont allow that , if we still try we will get compile error
        
        Logger log2 = *log1 ; // copy constructur invoked
        ** why copy constructor why not assignment ????
        compiler does this -> Logger log2(*log1)
        NOTE ******* -> whern ever there is object creation it is CONSTRUCTOR CALL SO COPY CONSTRUCTOR IS CALLED , and when the object is already there (UPDATE) then assignment is caleed

        Logger log3;
        log3 = *log1 ; // Assingment operator called since the log3 was already presnet there

   */

   Logger& operator=( const Logger &) ;

   public :
   
   /*
        why the funciot is static ?
        -> simply to allow this Logger::getInstance()
        ** only static methods can be called using the :: , but why ?
        ** when ever we write a non-static method in c++ , the compiler attaches a this pointer
        but while doing Logger::log("hello") , there is no object to pass as the this pointer . 
        the compiler doesn't know which memory to work on , so it doesn't allow this !
        
        when we do , myObj.log("hello") , the compiler converts it to log(&myObj , "hello")

        Logger::myfunc() -> calling -> myfunc() must be a static functin
        obj.mufunc() -> calling -> myfunc() can be static or non static
        Logger::SOME_VALUE -> accessing -> SO<E_VALUE is a static varialbe , enum or typedef
   */
    
   static Logger * getInstance () {
    /*
        why can't we return a object itself and why do we return a pointer 
        ****NOTE object creation in C++ 
            1. stack -> logger l '-;
            2. heap -> Logger* l = new logger () ;
        lets try to return the object directly
        static Logger getInstance () {
            Logger l ;
            return &l ; // return the address 
        }
        // but hey notice since the scope is exited l is destroyed
        // and l is now the address is pointing to nothing
        // calling any fucntion will give undefined error
    */
     // lazy initialization logic 
     if ( instance == nullptr ) { // not thread safe , if multiple threads hit the same condition multiple obects could be created 
        instance = new Logger() ;
     }
     return instance ; 
   }
   static void destroyInstance() {
        if (instance != NULL) {
            delete instance;  // Triggers the private destructor
            instance = NULL;  // Safety: Set to NULL so we don't point to dead memory
        }
    }

    void log(const char* msg) {
        std::cout << "[LOG] " << msg << "\n";
    }
};
// not doing this will cause the linker error
// undefined reference to `Logger::instacne`
// wen we do static Logger* instace is just a blue print , tell the compiler 
// the variable exista somewhere 
// we must explicityly define somewhere globally
// so that to actually allocate memory for the pointer itself

Logger * Logger::instance = nullptr ;

int main() {
    Logger::getInstance()->log("hello world ") ;
    Logger::destroyInstance(); // explicitely clear the memory
}