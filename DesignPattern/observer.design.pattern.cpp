#include <bits/stdc++.h> 
using namespace std  ; 

class StockObserver {
    public :
    virtual ~StockObserver () {} ;
    // the Subject calls this function only
    virtual void update ( string &stock_name , double price ) = 0 ; 
};

class StockSubject {
    public :
    virtual ~StockSubject() { } ;
    virtual void attach ( StockObserver * o) = 0 ;
    virtual void detach ( StockObserver * o) = 0 ;
    virtual void notify ( string &stock_name , double price ) = 0 ;
} ;

// concrete subject : publish prices 
class StockTicker : public StockSubject {
    private:
    // list of oberserver needing this service of price whenever there is an update 
    vector < StockObserver *> observers ; // non - owining ( no responsibility of the deleting ) , so that observers can still listen to the subject even if it dies 
    public:
    void attach ( StockObserver * o ) override {
        observers.push_back ( o ) ;
    }
    void detach ( StockObserver * o ) override {
        observers.erase(std::remove(observers.begin(), observers.end(), o), observers.end());
    }
    void notify ( string &stock_name , double price ) {
        for ( auto &o : observers ) {
            try {
                o -> update ( stock_name , price ) ;
            } catch (...) { // ellipsis 
                cerr << "[StockKicker] observer thew and error" ;
            }
        }
    }

    // API web socke that recieves these updates 

    void publishPrice ( string & stock_name , double price ) {
        cout << "[StockTicker] " << stock_name << " -> " << price << endl ; 
        notify ( stock_name , price ) ; 
    }
} ;

// concrete Observers 

class PriceDisplay : public StockObserver {
    public :
    void update ( string & stock_name , double price ) {
        cout << "[PriceDisplay] " << stock_name << " : $" << price << endl  ;

    }
};

class PriceAlert : public StockObserver {
    private : 
    double limit ; 
    public :
    PriceAlert ( double limit ) {
        this->limit = limit ; 
    }
    void update (  string &stock_name , double price ) {
        if ( price > limit ) {
            cout << "[PriceAlert] ALERT: " << stock_name << " reached " << limit << " (price=" << price << ")\n";
        }
    }
}

;
int main () {
    // subject we create on heap just for fun 
    StockTicker *ticker = new StockTicker() ;

    // observers we create on heap again for fun 
    PriceDisplay *display = new PriceDisplay () ;
    PriceAlert *alert = new PriceAlert(150) ;

    // subscribe 
    ticker->attach(display) ;
    ticker->attach(alert) ;

    //publish prices 
    string stock_name = "GOOLE" ;
    ticker->publishPrice( stock_name , 120.5) ;
    ticker->publishPrice (stock_name , 160 ) ;
    ticker->publishPrice ( stock_name , 120 ) ; 

    //



    /*
        why didn't we use the new keyword to initialise the object ?
        like we usually did ?
        PriceDisplay display ; 
        and not PriceDisplay *display = new PriceDislay() ;
        if we create it using the new and then do 
        ticker -> attach (display) , 
        now if ticker dies , display pointer is left standed and wont be deleted 
        might create issues later

        BUT WE CAN USE THE NEW KEYWORD WITHOUT ANY ISSUES 
    */
}


/*
    Observer Pattern
    ** when ever the subject updates , all the observers need to be identified
    ** eg -> react state update -> componenet re-render
          -> UI botton clicker money listners responds 
    ** Isubject (has-a list of observers ) -> concrete Subjects 
    ** Iobserver know the subject 
    
    why subject an interface ?
    => Because many kinds of subjects may exist , and observers shouldn't care which concrete subject they observe
    why observers an interface ? 
    => simimlar logic 

    why subject owns (passed by referece) the observer list ?
    => real time and dynamic 

    OWNERSHIP OF POINTERS IN GENERAL 
    -> prevents problems related to raw pointers , unintenal delete , read after delete , memory leaks 
    -> the one who is the owner of the pointer is responsible for deleting it 
    1. Exclusive ownership ( single owner ) 
      ** exactly one object is responsible for the lifetime of the pointer
      ** unique_ptr < T > 
      ** used when only one conponenet should manage the resource lifetime
    2. Shared Ownership ( multiple owners )
      ** multiple party co-own the objedt , it is destroyed only when the last of the owner releases it
      ** shared_ptr < T > 
      ** eg shared cache entries 
    3. Observing / non - owning references 
      ** pointer refers to an object owned elsewhere ( no delete ) 
      ** Use for temporory access or when lifetime is guranteed externally
    4. Weak Observation ( broken lifecycles / optional lifetimes ) 
      ** weat_ptr < T > observes a shared_ptr without extending lifetime and prevents cycle
      ** used for caches , observer list 


    *** new keyword in C++ 
        => allocated on heap 
        => ownerships 
            -> who owns this object ? and when will it be deleted 
                archetectural coupling 
*/