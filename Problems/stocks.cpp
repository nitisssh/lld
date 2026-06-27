#include <bits/stdc++.h>
using namespace std ;

// a kind of a parent class holds all the observers
// the moment anything changes it notifies all the observers by calling there some method



// different mediums to notify
class Ichannel {
    public:
    virtual ~Ichannel () = default ;
    virtual void notify(string name , int amount , string user ) = 0 ;
};

class SmsNotifier : public Ichannel {
    public:
    void notify(string name , int amount , string user ){
        cout << "USER: " << user << endl ;
        cout << "[SMS] Stock " << name << " price changed to " << amount << endl ;
    }
};

class Email : public Ichannel {
    public:
    void notify(string name , int amount , string user ){
        cout << "USER: " << user << endl ;
        cout << "[EMAIL] Stock " << name << " price changed to " << amount << endl ;
    }
};

// making it interface to allow multiple type of observer to be attached to the stocks not just users 
// it could be like usres and analytical service

class IObserver {
    public :
    virtual ~IObserver () = default; 
    virtual void notify ( string name , int amount ) = 0 ;
    virtual void addChannel ( Ichannel *c )  = 0 ;
};

// each user can be attached to multiple stocks 
// and user is a type of observer 
class User : public IObserver {
    private:
    string name ;
    // could use multiple channels to get notified 
    set < Ichannel * > channels ;// not everyone is going to use 
    // kind of violation of ISP but its a tradeoff 
    // other thing we could do is to remove it from here
    // and directly fill in constructure 
    // but again makes the code harder to comptem
    public:
    User ( string name ) {
        this -> name = name ;
    }
    void addChannel ( Ichannel * c ) {
        channels.insert ( c ) ;
    }

    void notify ( string name , int amount ) {
        for ( auto ch : channels  ) {
            ch -> notify ( name , amount , this->name ) ;
        }
    }
};

class Analytics : public IObserver {};
class Stock {
    public:
    string name ;  
    int price ;
    set < IObserver * > observers ;
   
    Stock (int price , string name ) {
        this -> price = price ;
        this -> name = name ;
    }
};

class StockManager {
  public:
    map < string , Stock* >  stocks;
    StockManager(){}

    void addStock (Stock *s ) {
        stocks[s->name] = s ;
    }
    void addSubscriber (IObserver* observer , string name ) {
        stocks[name]->observers.insert(observer ) ;
    }

    void unSubscribe (IObserver *observer , string name ){
        stocks[name]->observers.erase (observer) ;
    }
    void alterPrice (int amount ,string name ) {
        stocks[name]->price = amount ;
        for ( auto obs : stocks[name]->observers ) {
            obs -> notify (name , amount ) ;
        }
    }
};

int main () {
    Stock *a = new Stock (129 , "Coke") ;
    Stock *b = new Stock (1200 , "SpaceX") ;
    StockManager *manage = new StockManager() ;

    Ichannel *email = new Email () ;
    Ichannel* sms = new SmsNotifier() ;

    IObserver *alice = new User ("Alice" ) ;
    alice->addChannel (email) ;
    alice->addChannel (sms) ;

    IObserver *bob = new User ("Bob" ) ;
    bob->addChannel (email) ;

    manage->addStock (a) ;
    manage->addStock (b) ; 

    manage->addSubscriber(alice , a -> name ) ;
    manage->addSubscriber(bob , a -> name ) ;

    manage->alterPrice(135 , a -> name ) ;
    manage->alterPrice(139 , a -> name ) ;
    manage->alterPrice(89 , b -> name ) ;
};


// current issue , everyone is getting notified , but ideally only a single person should get notification

// current code acts like as if the notification is the observer and while its just the channel
// basically users should be the one who act as a observer 