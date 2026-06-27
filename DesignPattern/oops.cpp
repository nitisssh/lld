

// shallow copy and deep copy

// shallow copyt only copys the value but it keeps pointing to the same memory 
#include <bits/stdc++.h>
using namespace std ;

// class shallow {
//     public:
//     int *val ;
//     shallow() {}
//     shallow (shallow &s) {
//         int *vall = new int(*s.val) ;
//         this->val = vall ;
//     }
// };

// overloadingn is compile time polymorphism 
// method overloading and function overloading

class Vector {
    public:
    int x , y ; 
    Vector ( int x , int y ) {
        this -> x = x ; 
        this -> y = y ; 
    }
    // one operator is aways the this fucntion 
    // a + b -> compiler -> a.operator+(b)
    Vector operator+(Vector &rhs ) {
        Vector v(this->x + rhs.x , this->y + rhs.y) ;
        return v;
    }

    // Vector (Vector &rhs ) {
    //     Vector v ( rhs.x , rhs.y) ;
    // }
};

int main () {
    // shallow s ;
    // s.val = new int(100) ;
    // cout << *s.val << endl ;
    // shallow p = s ; // looks like a = operator but actually is a copy constructor(shallow)
    // // shallow s , shallow p -> s = p ? this is not assignment operator 
    // //p.val = new int(200) ;// makes p point to commpletely new memory loccation sinnce val is a ppointer 
    // *p.val = 200 ;
    // cout << *s.val << endl ;
    Vector a ( 4 , 5 ) , b ( 9, 7 ) ;
    Vector c = a + b ; 
    cout << c.x << " " << c.y << endl ;
}