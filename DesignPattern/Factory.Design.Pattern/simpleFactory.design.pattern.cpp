/*
    this basically not a design pattern but a principle(only talking about the simple)
    
    *** Why this design pattern ?
        -> earlier, whenever we needed any object we used to directly create it , in the middle of the 
        business logic , which is not eleganto 
        -> if somehow we had different type of products and each product with different type of arguments
        -> the code would become hella messy 
        -> so , we basically use this factory design pattern to create object and 
        thus seperate the business logic with the object creation

    ** 3 ways to implement them 
    1. simple factory
    2. Factory method (gof->gang of four) ( we have different type of factory ) 
    3. Abstract Factory (gof -> gang of four) ( we have different types of factory and different types of products )
*/



#include <bits/stdc++.h>
using namespace std ; 


enum class BeverageType {
    Espresso , 
    Latte , 
};

/*Product type1*/
class Beverage {
    public :
    virtual ~Beverage(){}
    virtual void getDescription() = 0 ;
};

class Espresso : public Beverage {
    public :
    void getDescription() override {
        cout << "[Espresso] Hello I am Espresso. " << endl ; 
    }
};

class Latte : public Beverage {
    public : 
    void getDescription () override {
        cout << "[Latte]  Hello I am Latte. " << endl ; 
    }
};

class BeverageFactory {
    public :
    Beverage* getBeverage ( BeverageType type ) {
        if ( BeverageType::Espresso == type ) {
            return new Espresso() ;
        } else if ( BeverageType::Latte == type ) {
            return new Latte() ;
        }
    }
} ;


int main () {
    BeverageFactory *bevFactory = new BeverageFactory() ;

    /* everytime we need a beverage we are creating a complete object
       could be expensive idk man much about it

    */

    Beverage *e = bevFactory->getBeverage(BeverageType::Espresso) ;
    e -> getDescription() ;

    Beverage *l = bevFactory->getBeverage(BeverageType::Latte) ;
    l -> getDescription() ;
}