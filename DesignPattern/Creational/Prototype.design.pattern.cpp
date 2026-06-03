/*
    allows us to create new objects by simply cloning existing ones , instead of intantiating from begin
    eg-> we want to spawn 100 bots in the frame , doing it from scratch will consume huge resoure

    Problems with naive copying ?
    -> why cant we just create a new object and manually copy the variables over ?
    -> multiple problems
        ** if class is interface you cant make its object
        ** if we try to create a concrete implementation , we will have to have lot of if-else
        ** private variables would be in accessible
    FIX: instead of having an external code copy the object
         let the object itself expose a functionn that lets you clone it

    WHAT MAKES THIS PROTOTYPE STRUCTURE WORK ?
*** polymorphism , *this pointer , copy constructor
    these three makes it work 
    polymorphism -> the client which needs to build the clones doesn't know if the enemy is orc or goblin 
                    it only know its a enemy , polymorphism lets us create different types of enemys without changhin the code 
                    for eg 
                    Monster * spawnMonster ( Monster * prototype ) {
                        return prototype->clone() ;
                    }    
                    // we dont need to know what the prototye actually is 
    *this pointer -> object simply derefernces itself
    copy constructor -> actual job of copying  
*/
#include <bits/stdc++.h>
using namespace std;


/* PROTOTYPE INTERFACE */

class Monster {
public:
    virtual ~Monster () {
        cout << " [Memory] Base Monster Destroyed " << endl ; 
    } ;
    virtual Monster* clone() const = 0 ; // const wont let us alter any data in this funciton 
    virtual void attack() const = 0 ; 
};

/* Comcrete Monsters Prototyes*/
class Orc : public Monster {
private : 
    string clanName ; 
    int health , damage ;
public :
    Orc ( string clan , int hp , int dmg ) {
        this->clanName = clan ;
        this->health = hp ; 
        this->damage = dmg ; 
        cout << " [System]  Heavy lifting : Loading Orc textures with " << clan << " " << hp << " " << dmg <<  "..." << endl ;
    }
    ~Orc() override {
        std::cout << "  [Memory] Orc destroyed.\n";
    }
    /*
        why return Monster* and not Monster object itself ?
        -> new allocates memory in heap ( ram ) and not on stack so it lives even after clone fucntion finished running
        -> *this derefernces the current Orc obect and passes it as argument
        -> this invoks the COPY CONSTRUCTOR which does the shallow copy
        -> shallow copy is okay here since no raw pointers are involved
    */
    Monster* clone() const override {
        return new Orc(*this) ;
    }
    void attack() const override {
        std::cout << "Orc from " << clanName << " clan attacks for " << damage << " damage! (HP: " << health << ")\n";
    }
};
class SacredWeapon {
public :
  string name ; 
  int damage ; 
  SacredWeapon(string namee , int dam ) : name(namee) , damage(dam) {

  }
};
class Goblin : public Monster {
private :
    string weapon ;
    SacredWeapon *secred_weapon ;  // pointer asset need to do deep copu
    int health ; 
public :
    Goblin(string weapon , int health) : weapon(weapon) , health(health) {
        secred_weapon = new SacredWeapon("Spiked Club" , 50 ) ;
        std::cout << "  [System] Heavy lifting: Loading Goblin textures with " << weapon << "...\n";
    }

    /* Deep Copy constructor */
    Goblin ( const Goblin& original ) {
        cout << "  [System] Performing DEEP COPY of Goblin...\n"; 
        /*copying the simple data normally*/
        this->health = original.health ; 
        this->weapon = original.weapon ;

        /*instead of copying the pointer create a brand new weapon*/
        this->secred_weapon = new SacredWeapon(*(original.secred_weapon)) ;
    }

    ~Goblin() override {
        delete secred_weapon; // Now safe, because every Goblin owns its very own Weapon memory!
        std::cout << "  [Memory] Goblin destroyed.\n";
    }

    // The Clone Method (Remains exactly the same syntax!)
    Monster* clone() const override {
        // This still looks like a shallow copy syntax, but because we wrote 
        // the custom Copy Constructor above, C++ routes this call through our DEEP COPY logic!
        return new Goblin(*this); 
    }
    void attack() const override {
        std::cout << "Goblin slashes with " << weapon << "!\n";
    }
};


// The Cient ( The Spawner/Registery )
class MonsterSpawner {
private:
    unordered_map < string , Monster*> prototypes ; 
public:
// Destructor: Because we hold raw pointers, the Spawner MUST clean them up
    // when the game closes, or the "Master Copies" will leak memory.
    ~MonsterSpawner() {
        std::cout << "\n--- SHUTTING DOWN SPAWNER (Cleaning Master Copies) ---\n";
        for (auto& pair : prototypes) {
            delete pair.second; // Frees the memory for each master prototype
        }
    }
    /*Register a new master copy . We pass a raw pointer */
    void learnMonster( string key, Monster* prototype ) {
        prototypes[key] = prototype ;
    }
    
    /* spawn a monster by cloning */
    Monster* spawnMonster(const std::string& key) {
        if (prototypes.find(key) != prototypes.end()) {
            // Returns a brand new pointer to a cloned object.
            // WARNING: The caller of this function is now responsible for deleting it!
            return prototypes[key]->clone();
        }
        std::cout << "Error: Monster prototype '" << key << "' not found!\n";
        return nullptr;
    }
};

int main () {
    MonsterSpawner spawner ; 
    std::cout << "--- INITIALIZATION PHASE (Happens Once) ---\n";
    // We use 'new' to allocate our master copies on the heap.
    // The spawner takes ownership of these specific pointers.
    spawner.learnMonster("BruteOrc", new Orc("Bloodfang", 500, 50));
    spawner.learnMonster("SniperGoblin", new Goblin("Poison Bow" , 89));
    std::cout << "\n--- GAMEPLAY PHASE (Spawning enemies) ---\n";
    // Spawning enemies. These are fresh pointers pointing to new memory.
    Monster* orc1 = spawner.spawnMonster("BruteOrc");
    Monster* orc2 = spawner.spawnMonster("BruteOrc");
    Monster* goblin1 = spawner.spawnMonster("SniperGoblin");

    // Action!
    orc1->attack();
    orc2->attack();
    goblin1->attack();

    std::cout << "\n--- CLEANUP PHASE (Manual Memory Management) ---\n";
    // Because we are not using smart pointers, we MUST manually delete 
    // every clone we created to give the memory back to the operating system.
    delete orc1;
    delete orc2;
    delete goblin1;

    // When main() ends, the 'spawner' goes out of scope, calling its destructor,
    // which in turn will de
}


// NAIVE COPYING NIGHTMARE 

// class Enemy {
// public:
//     virtual ~Enemy() = default ; 
//     virtual void attack () const = 0 ; // pure virtual function ( interface )
// };

// // the concrete class
// class Goblin : public Enemy {
// private:
// // private variables only the goblin can access dem
//     int health = 100 ; 
//     string weapon = "Rusty Dagger" ;
// public:
//     void attack() const override {
//         cout << "Goblin attacks with " << weapon << endl ;
//     }
// };
// // we wish to return the exact copy of the target
// Enemy* naiveDuplicate( Enemy * target ) {

//     // attempt one -> interface problem
//     // lets just create a new enemy
//     // Enemy * myCopy = new Enemy() ;
//     // above code gets compilation error -> saying can create object of abstract class

//     // attempt 2 -> "Class Level Dependency Problem" 
//     // since we are using better design , we passed Enemy* and not the concrete enemy 
//     // we will have to guess what kind of enemy it is , wheather it is a goblin , or a dragon
//     // lets go with goblin
//     Goblin* myGoblinCpy = new Goblin() ;
//     // but it is fucked if target were dragon 
//     // to fix this we will have to have multiple if-else 
//     // which will be hell 

//     // attempt 3 -> private variable issue in manually copying
//     // lets just say we guessed it , it was a goblin now 
//     Goblin * realTarget = dynamic_cast<Goblin*> (target) ; // force it to be goblin

//     // myGoblinCpy->health = realTarget->health , we can't access the private variables 

// }
// int main ( ) {
//     Enemy * myEnemy = new Goblin() ;
//     Enemy * clonedEnemy = naiveDuplicate(myEnemy) ;
// }


