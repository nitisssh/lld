#include <bits/stdc++.h>
using namespace std ;

int n , m , q ; 
int parent[200001] , rankk[200001] , depth[200001];
vector < vector < pair < int , int >>> adj ;
vector < vector < int >> jump  , mini ;

int find ( int node ) {
    if ( node == parent[node] ) {
        return node ;
    }
    return parent[node] = find ( parent[node] ) ;
}

bool unite ( int u , int v ) {
    int x = find (u) ;
    int y = find (v) ;
    if ( x == y ) return false ; 
    if ( rankk[x] < rankk[y] ) {
        swap ( x , y  ) ;
    } 
    parent[y] = x ;
    rankk[x] += rankk[y] ;
    return true ;
}

void dfs ( int node , int par , int w , int deep ) {
    jump[node][0] = par ;
    mini[node][0] = w ;
    depth[node] = deep ;
    for ( int power = 1 ; power < 22 ; power ++ ) {
        int mid = jump[node][power - 1] ;
        if ( mid != -1 ) {
            jump[node][power] = jump[mid][power - 1] ;
            mini[node][power] = max ( mini[node][power - 1] , mini[mid][power - 1]) ;
        }
    }
    for ( auto &x : adj[node] ) {
        if ( x.first != par ) {
            dfs ( x.first , node , x.second  , deep + 1) ;
        }
    }
}

int lca ( int u , int v ) {
    if ( depth[u] < depth[v] ) {
        swap ( u , v ) ;
    }
    int diff = depth[u] - depth[v] ;
    for ( int power = 0 ; power < 22 ; power ++ ) {
        if ( (diff >> power) & 1 ) {
            u = jump[u][power] ;
        }
    }
    
    if ( u == v ) {
        return u ; 
    }

    for ( int power = 21 ; power >= 0 ; power -- ) {
        if (jump[u][power] != jump[v][power] ) {
            u = jump[u][power] ;
            v = jump[v][power] ; 
        }
    }

    return jump[u][0] ;
}

int main () {
  cin >> n >> m ;
  adj.resize ( n + 1 ) ;
  jump.resize ( n + 1 , vector < int > ( 22 , -1 )) ;
  mini.resize ( n + 1 , vector < int > (22 , -1 )) ;
  for ( int i = 0 ; i < n + 1 ; i ++ ) {
    parent[i] = i ;
    rankk[i] = 1 ;
    depth[i] = 0 ;
  }
  long long sum = 0 ;
  vector < vector < int >> edges , copyy; 
  for ( int i = 0 ; i < m ; i ++ ) {
    int u , v , w ; 
    cin >> u >> v >> w ; 
    u -- , v -- ;
    edges.push_back ( { w , u , v } ) ;
  }
  copyy = edges;
  sort ( edges.begin() , edges.end() ) ;
  //reverse (edges.begin() , edges.end() ) ;

  for ( int i = 0 ; i < edges.size() ; i ++ ) {
    int u =  edges[i][1] , v = edges[i][2] , w = edges[i][0] ;
    if ( unite ( u , v )) {
        sum += w ;
        adj[u].push_back({v , w}) ;
        adj[v].push_back({ u , w}) ;
    } 
  }
  dfs ( 0 , -1 , (int)-1 , 0) ;

  for ( int i = 0 ; i < m ; i ++ ) {
    int u = copyy[i][1] , v = copyy[i][2] , w = copyy[i][0] ;
    int lcaa = lca ( u , v ) ;
    int distu = -(depth[lcaa] - depth[u]) ;
    int distv = -(depth[lcaa] - depth[v]) ;
    int minii = -1e9 ;
    for ( int power = 0 ; power < 22 ; power ++ ) {
        if ( (distu >> power ) & 1 ) {
            minii = max ( minii , mini[u][power]) ;
            u = jump[u][power] ;
        }
        if ( (distv >> power ) & 1 ) {
            minii = max ( minii , mini[v][power]) ;
            v = jump[v][power] ;
        }
    }
    cout << sum - minii + w << endl ;
  }
}