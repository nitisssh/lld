#include <bits/stdc++.h>
using namespace std;

class Beverage {
    public : 
        virtual string getDescription() = 0 ; 
        virtual double cost() = 0 ; 
        virtual ~Beverage() = default ; 
};

class Espresso : public Beverage {
    public :
        Espresso() {

        } 
        ~Espresso () {

        }
        string getDescription() override {
            return "This is an Espresso" ;
        }
        double cost() override {
            return 1.99 ;
        }
};

class DarkRoast : public Beverage {
    public :
        DarkRoast() {

        } 
        ~DarkRoast () {

        }
        string getDescription() override {
            return "This is a DarkRoast " ;
        }
        double cost() override {
            return 0.99 ;
        }
};

/* Abstract Decorators */
class CondimentDecorators : public Beverage { // .. is a component 
    protected : 
        Beverage *b ; // has a component
    public : 
        CondimentDecorators ( Beverage *b )  {
            this-> b = b ;
        }
        ~CondimentDecorators(){}

};

class Mocha : public CondimentDecorators {
    public :
    Mocha ( Beverage * b ) : CondimentDecorators(b) {} 

    string getDescription ()  override {
        return b -> getDescription() + " + Mocha " ;
    }

    double cost () override {
        return b -> cost () + 0.2 ; 
    }
};

class Whip : public CondimentDecorators {
    public : 
    Whip ( Beverage * b ) : CondimentDecorators(b) {}
    string getDescription () {
        return b -> getDescription() + " + Whip " ;
    }
    double cost () override {
        return b -> cost() + 0.5 ; 
    }
};


int main () {
    // lets first create the beverage of our choice 
    Beverage *b = new Espresso() ; // this is an espresso 
    cout << "I am Beverage b : " << b -> getDescription() << " with Cost: " << b -> cost () << endl ; 
    // make it mocha
    b = new Mocha (b) ; 
    cout << "I am Beverage b : " << b -> getDescription() << " with Cost: " << b -> cost () << endl ; 
    
    // adding whip over the mocha
    b = new Whip(b) ;
    cout << "I am Beverage b : " << b -> getDescription() << " with Cost: " << b -> cost () << endl ; 

    // leets try some other combination ?
    // whip then add mocha eww
    Beverage *c = new Espresso() ;
    cout << "I am Beverage c : " << c -> getDescription() << " with Cost: " << c -> cost () << endl ; 
    c = new Whip(c) ;
        cout << "I am Beverage c : " << c -> getDescription() << " with Cost: " << c -> cost () << endl ; 
    c = new Mocha(c) ;
        cout << "I am Beverage c : " << c -> getDescription() << " with Cost: " << c -> cost () << endl ;

    // try only whip and the Espresso 

    Beverage *d = new Espresso() ;
    cout << "I am Beverage d : " << d -> getDescription() << " with Cost: " << d -> cost () << endl ; ;
    d = new Whip(d) ;
    cout << "I am Beverage d : " << d -> getDescription() << " with Cost: " << d -> cost () << endl ; ;

    /*
        now imagine that , decorators were not a Component itself 
        then we would do something like this 
            Beverage *b = new Espresso() ;
            Mocha *mocha = new Mocha (b) ;
            Whip *whip = new Whip (mocha) ;

        but this would lead to explosion of classes
            Mocha *mochaWhip = new Mocha ( new Whip ( new Espresso() ) ) ; // would not be possible
    */
}

/*
    * decorator is a componenet and has the componnent as well 
    * basically decorator implments/extends the Component
    * decorator has a pointer or reference to the Componnent
    * ConcreteDecorators override the behaviour and forward calls to hte wrapped Componenet 
    
    * each decorator is layer of an onion . We call on the outermost layer and passes through each ayer and toward the core ConcreteComponent .
    * each layer can add its own behaviour before or after forwarding the call to the next layer.
*/ 
/*
     * Why this structure of is and has a Component ?
        -> Decorator is a Component 
            *** we just can wrap the behavioius on top of the core object 
                this is the main issue that is-a relationship solves 
                for different types we could have created different decorators
                but that would lead to explosion of classes , but still okay
                but the differnte combination of decorators would lead to exponential explosion of classes
                to overcome this we use the is-a relationship , so that we can wrap 
                and treat everything a Conponent

            ** to ensure that decorators can be used interchangeble by Components
               something like liskov substitution principle 
               Anything that expects a Component should be able accespt the core object or any decorated version of it

               we would also loose recursive wrapping of decorator 

            for eg ; 
                imagine there is no , extension 
                decorator is not a component and it doesnt extend or implement iti 
                then , 
                Beverage *beverage = new Espresso() ;
                Mocha *mocha = new Mocha(beverage) ;
                Whip *whip = new Whip(mocha) ;

                client code must know about all the concrete decorators implementation so as to use them  and use them directly 
                we cant do this , 
                Beverage *beverage = new Mocha (new Espresso()); in reality , we would simply delegate the request of wrapping to the fucntion which would return a Beverage pointer , hence the there is a decoupling
                because Mocha is not a Beverage
        -> Decorator has a Component
            ** simply to allow runtime capability of adding or removing responsibilites to the core object
            ** we can wrap or unwrap decorators around the core object dynamically at runtime
*/