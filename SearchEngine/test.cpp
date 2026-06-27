#include <bits/stdc++.h>
using namespace std ;

int main (){
    vector v = { 1 , 5, 6 , 11 , 56 , 88} ;
    int value = 6 ; 
    int low = 0 , high = v.size() - 1 ; 
    while ( low <= high ) {
        int mid = ( low + high ) >> 1 ; 
        if (  v[mid] == value ) {
            cout << "mid : " << mid << endl ; 
            break ;
        }
        if ( v[mid] > value ) {
            high = mid - 1 ;  
        } else {
            low = mid + 1 ; 
        }
    }
    
}