#include <bits/stdc++.h>
using namespace std ;

class VendingMachine ;

class IMachineState {
    public:
    // at any state one of these actions willl be perfomed 
    // NOTE : we will not really be implementing all the methods
    // in all the states , but its a kind of trade of we will have to take
    // here or else it will result in micro interfaces 
    // these state should retun a state itself when some action is perfomed on them 
    virtual IMachineState* insertCoin ( VendingMachine *v , int coin ) = 0 ;
    virtual IMachineState* selectItem ( VendingMachine *v ) = 0 ;
    virtual IMachineState* dispense ( VendingMachine *v ) = 0 ;
    virtual IMachineState* refill ( VendingMachine *v , int items ) = 0 ;
};

// concrete states 
class NoCoinState : public IMachineState {
    public:
    // i need to retun other state from a given state
    // for this all the state will have to carry all the dependent state
    // very much tightly coupled it will be 
    // lets introduce a daddy class that will hold all the concrete states 
    NoCoinState(){}
    IMachineState* insertCoin ( VendingMachine *v , int coin ) override ;
    IMachineState* selectItem ( VendingMachine *v ) { return this;}
    IMachineState* dispense ( VendingMachine *v ) { return this ;}
    IMachineState* refill ( VendingMachine *v , int items) { return this ;}
};


class HasCoinState : public IMachineState {
    public:
    HasCoinState(){}
    IMachineState* insertCoin ( VendingMachine *v , int coin ) override ;
    IMachineState* selectItem ( VendingMachine *v ) override ;
    IMachineState* dispense ( VendingMachine *v ) { return this ;}
    IMachineState* refill ( VendingMachine *v , int items ) { return this ;}
};

class DispenseState : public IMachineState {
    public:
    DispenseState() {}
    IMachineState* insertCoin ( VendingMachine *v , int coin ) {
        return this ;
    }
    // one coin per item 
    IMachineState* selectItem ( VendingMachine *v ) {
        return this ;
    }
    IMachineState* dispense ( VendingMachine *v ) override ;
    IMachineState* refill ( VendingMachine *v , int items ) { return this ;}
};

class SoldOutState : public IMachineState {
    public:
    SoldOutState() {}
    IMachineState* insertCoin ( VendingMachine *v , int coin ) {
        return this ;
    }
    // one coin per item 
    IMachineState* selectItem ( VendingMachine *v ) {
        return this ;
    }

    IMachineState* dispense ( VendingMachine *v ) { 
        return this ;
    }
    IMachineState* refill ( VendingMachine *v , int items) override ;
};



// we can have only a single type of elemenet
class VendingMachine {
    public:
    int tot_coins = 0 ;
    int tot_items = 0 ;
    VendingMachine ( int tc , int ti ) {
        tot_coins += tc ;
        tot_items += ti ;
        if ( tot_items ) {
            curr_state = new NoCoinState() ;
        } else {
            curr_state = new SoldOutState() ;
        }
    }
    IMachineState *curr_state ; 
    // IMachineState *noCoinState = new NoCoinState() ;
    // IMachineState *hasCoinState = new HasCoinState() ;
    // IMachineState *dispenseState = new DispenseState() ;
    // IMachineState *soldOutState = new SoldOutState() ;

    void insertCoin ( int coin ) {
        curr_state = curr_state -> insertCoin ( this , coin ) ; 
    }

    void selectItem () {
        curr_state = curr_state -> selectItem (this) ; 
    }

    void dispense () {
        curr_state = curr_state -> dispense (this); 
    }

    void refill ( int items ) {
        curr_state = curr_state -> refill (this , items); 
    }
};

IMachineState* NoCoinState :: insertCoin(VendingMachine *v , int coin ) {
    cout << "YEAH the coins are inserted , tot_coins : " << coin << " tot-items :" << v->tot_items << endl ;
    v -> tot_coins += coin ;
    return new HasCoinState() ;
}

IMachineState* HasCoinState :: insertCoin(VendingMachine *v , int coin ) {
    v -> tot_coins += coin ;
    cout << "I already had coins , tot_coins : " << v -> tot_coins << " tot-items :" << v->tot_items << endl ; ;
    return new HasCoinState() ;
}

IMachineState* HasCoinState :: selectItem (VendingMachine *v ) {
    cout << "YEAH items selected " << " tot-items :" << v->tot_items << endl ; ;
    return new DispenseState() ;
}

IMachineState* DispenseState :: dispense (VendingMachine *v ) {
    if (v->tot_items - v->tot_coins == 0 ) {
        v->tot_coins = 0  ;
        v->tot_items -= v->tot_coins;
        // addtional logic could be there to have a return money state ... but let it go
        cout << "All Items consumed moving to the soldout state" << endl ;
        return new SoldOutState() ;
    }
    cout << "Transaction done moving to the noCoinState " << endl ;
    return  new NoCoinState();
}

IMachineState* SoldOutState :: refill(VendingMachine *v , int items ) {
    cout << "I am refilled ready to work , moving to the nocoin state " << endl ;
    v -> tot_items += items ;
    return new NoCoinState();
}

int main() {
    VendingMachine *v = new VendingMachine(0 , 2) ;

    v -> insertCoin(1) ;
    v -> selectItem() ;
    v -> dispense() ;
    
    cout << endl ;

    v -> insertCoin(1) ;
    v -> selectItem() ;
    v -> insertCoin(0) ;
    
    cout << endl ;
    
    v -> insertCoin(1) ;
    v -> selectItem() ;
    v -> dispense() ;
}