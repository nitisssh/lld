#include <bits/stdc++.h>
using namespace std ;

enum class OrderStatus {
    PLACED,
    CONFIRMED,
    PREPARING,
    READY_FOR_PICKUP,
    OUT_FOR_DELIVERY,
    DELIVERED,
    CANCELLED
};

string statusToString(OrderStatus status) {
    switch(status) {
        case OrderStatus::PLACED: return "Placed";
        case OrderStatus::CONFIRMED: return "Confirmed";
        case OrderStatus::PREPARING: return "Preparing";
        case OrderStatus::READY_FOR_PICKUP: return "Ready for Pickup";
        case OrderStatus::OUT_FOR_DELIVERY: return "Out for Delivery";
        case OrderStatus::DELIVERED: return "Delivered";
        case OrderStatus::CANCELLED: return "Cancelled";
    }
    return "Unknown";
}

//NOTIFICATIONS ( OBSERVSER DESIGN PATTERN )
class IOrderObserver {
    public:
    virtual void update( string orderId , OrderStatus status )= 0 ;
    virtual ~IOrderObserver() {} ; // if this dies the children also dies
};

// Payments Strategy pattern

class IPaymentStrategy {
    public:
    virtual bool processPayment(double amaount ) = 0 ;
    virtual ~IPaymentStrategy() {}
};

class CreditCardPayment : public IPaymentStrategy {
    private:
    string cardNumber ; 
    public:
    CreditCardPayment(string num): cardNumber(num){}
    bool processPayment(double amount) override {
        std::cout << "[Payment] Processing $" << amount << " via Credit Card ending in " 
                  << cardNumber.substr(cardNumber.length() - 4) << "\n";
        return true;
    }
};

class UPIPayment : public IPaymentStrategy {
private:
    string upiId;
public:
    UPIPayment(string upi) : upiId(upi) {}
    bool processPayment(double amount) override {
        cout << "[Payment] Processing $" << amount << " via UPI ID: " << upiId << "\n";
        return true;
    }
};

//DOMAIN ENTITIES
class MenuItem {
    private:
    string id , name ; 
    double price ; 
    bool isAvailable;
    public:
    MenuItem(std::string id, std::string name, double price, bool available = true)
        : id(std::move(id)), name(std::move(name)), price(price), isAvailable(available) {}
    string getId (){
        return id ;
    }
    string getName() {
        return name ;
    }
    double getPrice() {
        return price ;
    }
    bool getAvailable() const { return isAvailable; }
    void setAvailibility (bool flag){ isAvailable = flag;}
};

class Menu {
    private:
    unordered_map < string , MenuItem*> items;
    mutable mutex menuMutex ;
    public:
    // Ownership: Menu owns the MenuItem. Must delete all the allocated items.
    ~Menu() {
        for (auto &pair : items ) {
            delete pair.second ;
        }
    }
    void updateAvailibility(string item_id , bool flag ) {
        lock_guard<mutex>lock(menuMutex) ;
        if ( items.find(item_id) != items.end() ) {
            items[item_id]->setAvailibility(flag) ;
        }
    }
    void addItem (MenuItem* item) {
        lock_guard<mutex>lock(menuMutex) ;
        items[item->getId()] = item;
    }
    MenuItem* getItem (string &itemId ) {
        lock_guard<mutex> lock(menuMutex) ;
        if ( items.find(itemId) != items.end() ) {
            return items[itemId] ;
        }
    }
    void displayMenu() const {
        std::lock_guard<std::mutex> lock(menuMutex);
        std::cout << "\n--- Menu ---\n";
        for (const auto& pair : items) {
            MenuItem* item = pair.second;
            std::cout << "[" << item->getId() << "] " << item->getName() 
                      << " - $" << item->getPrice() 
                      << " (" << (item->getAvailable() ? "Available" : "Out of Stock") << ")\n";
        }
    }
};

class Restaurant : public IOrderObserver {
    private :
    string id ; 
    string name ;
    Menu* menu; // owns the menu 
    bool isActive ;
    public:
    Restaurant(string id , string name ) : id(id), name(name){
        menu = new Menu() ;
    }
    ~Restaurant(){
        delete menu ;
    }
    string getId() {return id ;}
    string getName() {return name ;}
    Menu * getMenu() {return menu;}
    void update (string orderId , OrderStatus status ) override {
        cout << "[Notification -> Restaurant " << name << "] Order " << orderId 
                  << " status changed to: " << statusToString(status) << "\n";
    }
};

//Actors (Users)
class User {
    protected:
    string id , name , phone ;
    public:
    User (string id, string name , string phone) : id(id) , name(name) , phone(phone){}
    virtual ~User(){} // so that when we create a child object using the base class ponter , calling delete on that pointer make sures the 
                        // childs destructor runs first 
    string getId() {return id ;}
    string getName() {return name ;}  
      
};

class DeliveryAgent : public User , public IOrderObserver {
    private:
    bool isAvailable;
    public:
    DeliveryAgent(std::string id, std::string name, std::string phone)
        : User(std::move(id), std::move(name), std::move(phone)), isAvailable(true) {}

    bool getIsAvailable() const { return isAvailable; }
    void setAvailable(bool available) { isAvailable = available; }

    void update(string orderId, OrderStatus status) override {
        cout << "[Notification -> Agent " << name << "] Update on assigned order " << orderId 
                  << ": " << statusToString(status) << "\n";
    }
};

class Customer : public User, public IOrderObserver {
public:
    Customer(std::string id, std::string name, std::string phone)
        : User(std::move(id), std::move(name), std::move(phone)) {}

    void update(string orderId, OrderStatus status) override {
        cout << "[Notification -> Customer " << name << "] Your order " << orderId 
                  << " is now: " << statusToString(status) << "\n";
    }
};

class OrderItem {
    public:
    MenuItem *item ;
    int quantity ;
    OrderItem ( MenuItem *item , int quantity ): item(item) , quantity(quantity){}
};

class Order {
    private:
    string orderId ;
    Customer* customer ;
    Restaurant* restaurant ;
    DeliveryAgent* agent ;
    vector < OrderItem > items ;
    OrderStatus status ;
    double totalAmount;

    vector < IOrderObserver*> observers ;
    mutable mutex orderMutex ;

    void notifyObservers () {
        for (IOrderObserver* observer : observers ) {
            observer->update(orderId  , status) ;
        }
    }

    public:
    Order(std::string id, Customer* cust, Restaurant* rest,  std::vector<OrderItem>& ordItems)
        : orderId(std::move(id)), customer(cust), restaurant(rest), agent(nullptr),
          items(ordItems), status(OrderStatus::PLACED), totalAmount(0.0){
        
        calculateTotal();
        addObserver(customer);// register observers
        addObserver(restaurant);
    }
    void addObserver(IOrderObserver* observer) {
        std::lock_guard<std::mutex> lock(orderMutex);
        if (observer != nullptr) {
            observers.push_back(observer);
        }
    }
    void assignAgent(DeliveryAgent* agent ) {
        this->agent = agent ;
        if ( agent != nullptr ) {
            agent->setAvailable(false) ;
            observers.push_back(agent) ;
            cout << "[System] Assigned Agent " << agent->getName() << " to Order " << orderId << endl ;
        }
    }
    void calculateTotal () {
        totalAmount = 0.0 ;
        for ( auto order : items ) {
            totalAmount += order.item->getPrice() * order.quantity ; 
        }
    }
    double getTotalAmount() const { return totalAmount; }
    string getOrderId() const { return orderId; }

    void setStatus (OrderStatus status ) {
        lock_guard<mutex> lock(orderMutex) ;
        this->status = status ;
        notifyObservers() ;
        if ( status == OrderStatus::DELIVERED or status == OrderStatus::CANCELLED) {
            if ( agent != nullptr ) {
                agent -> setAvailable(true) ;
            }
        }
    }
};

// FACADE

class Swiggy {
    private:
    unordered_map < string , Restaurant*> restaurants;
    unordered_map < string , DeliveryAgent*> agents ; 
    unordered_map < string , Customer*> customers ; 
    unordered_map < string , Order*> orders ;

    mutex sysMutex ;

    public:
    ~Swiggy() {
        for (auto& pair : orders)      delete pair.second;
        for (auto& pair : restaurants) delete pair.second;
        for (auto& pair : agents)      delete pair.second;
        for (auto& pair : customers)   delete pair.second;
        
        orders.clear();
        restaurants.clear();
        agents.clear();
        customers.clear();
        std::cout << "[System Destructor] All raw pointers cleanly deallocated.\n";
    }

    void registerCustomer(Customer* cust) {
        std::lock_guard<std::mutex> lock(sysMutex);
        customers[cust->getId()] = cust;
    }

    void registerRestaurant(Restaurant* rest) {
        std::lock_guard<std::mutex> lock(sysMutex);
        restaurants[rest->getId()] = rest;
    }

    void registerAgent(DeliveryAgent* agent) {
        std::lock_guard<std::mutex> lock(sysMutex);
        agents[agent->getId()] = agent;
    }

    Restaurant* getRestaurant(const std::string& id) {
        std::lock_guard<std::mutex> lock(sysMutex);
        auto it = restaurants.find(id);
        return (it != restaurants.end()) ? it->second : nullptr;
    }

    Order* placeOrder( string orderId , Customer* customer , Restaurant * restaurant , vector <OrderItem>&items , IPaymentStrategy* paymentMethod ) {
        if (customer == nullptr || restaurant == nullptr || paymentMethod == nullptr) {
            delete paymentMethod; // Clean up input pointer on failure
            return nullptr;
        }
        // check availivtly 
        for (auto itemm : items ) {
            if (itemm.item == nullptr or itemm.item->getAvailable() == false ) {
                cout << "[Error] Item is null or unavailable. Order failed.\n";
                delete paymentMethod;
                return nullptr ;
            }
        }
        // create order 
        Order* order = new Order(orderId, customer , restaurant , items);
        // process payment
        if (!paymentMethod->processPayment(order->getTotalAmount())) { // this wil do the payment anyways in this call
            std::cout << "[Error] Payment failed for order " << orderId << "\n";
            delete order;         // Clean up newly created order
            delete paymentMethod; // Clean up strategy pointer
            return nullptr;
        }

        delete paymentMethod ;

        // register order safely
        {
            lock_guard < mutex > lock(sysMutex) ;
            orders[orderId] = order ;
        }
        // update the status to confirmed since the payement is done
        order -> setStatus (OrderStatus::CONFIRMED) ;
        // assign order
        assignNearestAgent(order) ;
        return order ;
    }
    private:
    void assignNearestAgent(Order *order) {
        lock_guard<mutex> lock(sysMutex) ;
        for ( auto pair : agents ) {
            if ( pair.second->getIsAvailable()) {
                order->setStatus(OrderStatus::OUT_FOR_DELIVERY) ;
                order->assignAgent(pair.second) ;
                return ;
            }
        }std::cout << "[System Warning] No agents currently available for Order " << order->getOrderId() << "!\n";
    }
}

;
int main() {
    Swiggy swiggy ;
    Customer* customer = new Customer("C1", "Alice", "9876543210");
    DeliveryAgent* agent = new DeliveryAgent("A1", "Bob", "5556667777");
    Restaurant* restaurant = new Restaurant("R1", "Pizza Palace");

    // Pass ownership to the global system registry
    swiggy.registerCustomer(customer);
    swiggy.registerAgent(agent);
    swiggy.registerRestaurant(restaurant);

    // 2. Setup Menu
    MenuItem* item1 = new MenuItem("M1", "Margherita Pizza", 12.99);
    MenuItem* item2 = new MenuItem("M2", "Garlic Bread", 4.99);

    // Pass ownership of items to the restaurant's menu
    restaurant->getMenu()->addItem(item1);
    restaurant->getMenu()->addItem(item2);

    restaurant->getMenu()->displayMenu();

    vector < OrderItem > cart = {
        OrderItem(item1 , 2) ,
        OrderItem(item2 , 1) 
    };

    Order *order = swiggy.placeOrder("ORD1001" , customer , restaurant , cart , new UPIPayment("alice@okaysbi"));
    if (order != nullptr) {
        std::cout << "\n---> Updating Order States...\n";
        order->setStatus(OrderStatus::PREPARING);
        order->setStatus(OrderStatus::READY_FOR_PICKUP);
    }
    return 0 ;
}






/*
    Requirements 
    The food delivery service should allow customers to browse restaurants, view menus, and place orders.
    Restaurants should be able to manage their menus, prices, and availability.
    Delivery agents should be able to accept and fulfill orders.
    The system should handle order tracking and status updates.
    The system should support multiple payment methods.
    The system should handle concurrent orders and ensure data consistency.
    The system should be scalable and handle a high volume of orders.
    The system should provide real-time notifications to customers, restaurants, and delivery agents.

*******************************************************************************************************************************
    Solutions
    
    **ENTITIES ( nouns(potential entities) and verbs ( potential behaviour))
    **** NOUNS -> Food Delivery Service , Customers , Restaurants , Menus , Prices , Availaibility ,  Orders 
                    Delivery Agents , order tracking/status, Notifications , Payment Methods
    **** Verbs -> browse restaurantatns, view , menus , place orders
                 manage , place , accept /fullfill , tracking , status updates , support mulitple payment methods , 
                 provide real time notifications 

**********************************************************************************************************************************
    ** Nouns to entity 
        -> we need to ask three questions 
        1. is it a global boundary ( container vs contents ) ? -> YES: its a system/facade
        2. is it just a primitive value (5$ price tag vs "availaible")? -> YES: its a attribute 
        3. does it have distinct lifecycle and identity ? -> YES: its a entity
            ..if a customer changes its phone number it is still the same customer
            ..if a customer goes through phases , eg -> placing orders , tracking ..
    

*********************************************************************************************************************************

    ** FACADES -> Food Delivery Service 
    ** Attributes -> Prices , Availibility , Status (doesn't exist on there own)
    ** Entities -> Customer , Restaurant , Delivery Agent , Menu , Order , Payment Methods 

    **** other entities are added based on the usecases

************************************************************************************************************************************************

    ***** ENTITIES
    Customer -> customer_id , name ( string ) , phone_number ( string ) INDEXED , email , created_at 
            since we can have multiple address for a single user , we will make its own entity 
    CustomerAddress -> address_id (PK) , customer_id ( FK ) , label (eg home , office ..) , street_address , lattitue , longitude , is_dafaut
    Restaurant -> restaurant_id (PK) , name , is_active , lattitue , longitude
    RestaurantAnalytics -> orders_fullfiled, avg orders , rating time etc
    NOTE-> Creating a table for this is a nightmare , locks issue , since it needs updates , this db will overwhelm by locks and cpu spikes , db connection pool exhaustions
            Solution -> Event Driver CQRS ( command query responsiblity segregation )
                        after the delivery marks the order as delivered ,
                        we mark the order table as delivered 
                        and push the detail to a kafka queue
                        then we keep two subscriber each reading from the same topic
                        one will first keep the warm cache for readings
                        and the other then update the restaurant_analytics table using batch
                        by first keeping into in memory data in batch then filling the db
    Menu-> menu_id (PK) , Restaurant_id (FK) , last_updated
    MenuItem -> item_id (pk) , menu_id (fk) , name , price , is_available , version(optimistiv locking to avoid concurrent overselling)

    Order -> order_id ( pk ) , idempotency_key ( unique index , prevents dupliate order creation in case of network retries)
                customer_id (fk) , restaurant_id (fk) , agent_id ( fk) 
                order_status ( ENUM -> PLACED , CONFIRMED , PREPARING , PICKED_UP , DELIVERED , CANCELLED )
                item_total , delivery_fee , grand_total, created_at , version ( protects concurrent status transitions )
    OrderItem -> order_item_id (pk) , item_id ( fk ) , order_id (fk) , quantity , price_at_purchase
    PaymentTransaction -> transaction_id(pk) , order_id (fk) , amount , payment strategy , gateway_reference_id , timestamp

*********************************************************************************************************************************
    **CLASSES
    RULE-> Does this Concept encapsulate both State(data/attributes) and Behaviour(logic/rules)
           or does it represent a distinct responsibility that might change independently over time ?

*/