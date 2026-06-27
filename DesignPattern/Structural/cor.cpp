#include <bits/stdc++.h>
using namespace std ; 

// i wanna do one step after another
// kinda like what we do in the state pattern but in state pattern we return complete object
// a complete new state tht behaves differently

// in cor we simply follow a chain we dont really return a obje
class IdispenseHandler {
    public:
    virtual bool dispense (int amount ) = 0 ;
};

class thandler : public IdispenseHandler {
    public:
    int count = 0 ;
    IdispenseHandler *next;
    thandler( int count , IdispenseHandler *next ) {
        this -> next = next  ;
        this -> count = count ;
    }

    bool dispense (int amount ) {
        int need = amount / 1000 ;
        need = min ( need , count ) ;
        if ( need * 1000 == amount ) {
            count -= need ;
            cout << "[1000] amount handled by thandler " << need * 1000 << endl ;
            return true;
        }
        if ( next == nullptr ) {
            cout << "[1000]  failed " << endl ;
        }
        if ( next -> dispense (amount - need * 1000 )) {
             cout << "[1000] amount handled by thandler " << need * 1000 << endl ; 
            count -= need ;
            return true ;
        }
        cout << "[1000]  failed " << endl ;
        return false ;
    }
};

class fhandler : public IdispenseHandler {
    public:
    int count = 0 ;
    IdispenseHandler *next;
    fhandler( int count , IdispenseHandler *next ) {
        this -> next = next  ;
        this -> count = count ;
    }

    bool dispense (int amount ) {
        int need = amount / 500 ;
        need = min ( need , count ) ;
        if ( need * 500 == amount ) {
            count -= need ;
            cout << "[500] amount handled by thandler " << need * 500 << endl ;
            return true;
        }
        if ( next == nullptr ) {
            cout << "[500]  failed " << endl ;
        }
        if ( next -> dispense (amount - need * 1000 )) {
            count -= need ;
            return true ;
        }
        cout << "[500]  failed " << endl ;
        return false ;
    }
};

class Atm {
    public:
    IdispenseHandler *handle;
    Atm (IdispenseHandler *handle) {
        this -> handle = handle ;
    }
    
    void dispense ( int amount ) {
        handle -> dispense (amount ) ;
    }
};

int main () {
    IdispenseHandler *f = new fhandler (10000 , nullptr) ;
    IdispenseHandler *t = new thandler ( 10000 , f ) ;
    Atm *atm = new Atm (t) ;
    atm->dispense (1000000) ;
}