#include <bits/stdc++.h>
using namespace std ; 

// to avoid the cache misses we have flattene the hsnw 
class FlatHSNW {
private :
    int dimensions;
    int M ; // no of max connections for layers other than the 0th ( most dense)
    int M0 ; // usuall 2 * M , we want to find the exact nodes 
    // since the hurestic we use to avoid a edge with the new node , if it has edge with other node connected to the new node
    // and smaller length , to avoid forming isolated cluster and keep connections with outer layer
    // we make sure we have high conncetivity to narrow down the search accurately
    int ef_construction; // explore factor while edge construction during the insertion 
    // if this value is high we have better nodes to connect to and we will have nice answers on searching
    // but keeping this value makes indexing very hard 
    int max_layers ; // we will have 4 , ( 0 - 4 )
    int entry_node ; 
    int highest_level ;
    int curr_node_count ; 
    float mL ;
    mt19937 rng ;// random number generator
    uniform_real_distribution<float> uniform_dist ; // uniform distribution in range a to b
    // flat memories 
    vector < float > all_embeddings ; // all nodes embedding in sequential order in the memory to reduce cache miss 
    vector < int > all_links ; // all nodes sequentially , 
    // it contains the following info sequentially in the memory for each node
    // for each node 
    /*
        {
           max_layer -> the layer at which this node will be inserted , found using log 
           next max_layers no of slots of ( letss say int ) .. to store the no of neighbour connected in each layer
           L0 --> neigh ids
           L1 --> negih ids
           .. till L4

           a node might not have connection to all the layers but still we reserve the space 
        }
    */
    int link_stride ; // space each node takes -> here 20 for each node 
    

    // memory access helpers 
    // starting pointer of the embedding
    float* get_vector ( int id ) {
        return &all_embeddings[id * dimensions] ;
    }
    // for the given node give its max layer pointer
    int* get_node_max_layer_ptr ( int id ) {
        return &all_links[id * link_stride] ;
    }

    int* get_node_connections_start_ptr ( int id , int layer ) {
        if ( layer > 0 ) { // since the 0th layer has more connectiond allowed
            return &all_links[id * link_stride + ( 1 + max_layers ) + (layer - 1) * M + M0] ;
        } else {
            return &all_links[id * link_stride + ( 1 + max_layers )] ;
        }
    }

    int get_random_layer() {
        float r = uniform_dist(rng) ;
        if ( r == 0.0f )  r = 0.000001f ;
        int lyr = floor(-log(r) * mL ) ;
        return min ( lyr , max_layers - 1);
    }

    // euclidean distance between node and node_id
    float distance ( vector < float > & query , int entry_point ) {
        float* v1 = get_vector ( entry_point ) ;
        float dist = 0 ;
        for ( int i = 0 ; i < dimensions ; i++ ) {
            dist += ( query[i] - v1[i] ) * ( query[i] - v1[i] ) ;
        }
        return sqrt(dist) ;
    }

    float distance(float* v1, float* v2) {
        float dist = 0.0f;
        for (int i = 0; i < dimensions; ++i) {
            float diff = v1[i] - v2[i];
            dist += diff * diff;
        }
        return sqrt(dist);
    }

    // in a given layer with starting point as entry_node  find me the nearest node to this 
    // have ef no of nodes in considerationn at any point
    vector < pair < float , int> > search_layer ( vector <float> & query , int entry_point , int ef , int layer ) {
        multiset < pair < float , int >> candidates ; 
        multiset < pair < float , int >> result_queue ;
        set < int > visited ; 

        float initial_dist = distance ( query , entry_point ) ;
        candidates.insert({initial_dist , entry_point}) ;
        visited.insert(entry_point) ;
        result_queue.insert({initial_dist , entry_point});
        while ( candidates.size() ) {
            auto [dist , node_id] = *candidates.begin() ;
            candidates.erase(candidates.begin()) ;
            if ( dist > (--result_queue.end())->first ) {
                break ;
            }
            // get the connections of the current node in the given layer 
            int * layer_connections_start_ptr = get_node_connections_start_ptr(node_id, layer) ;
            vector < int > connections ;
            int size = M0 ;
            if ( layer > 0 ) {
                size = M ;
            }
            // Style-Fix: Changed 'src' to 'node_id' to resolve undefined compile error
            int *nbr_counts = &all_links[node_id * link_stride + 1 + layer ] ;
            for ( int i = 0 ; i < *nbr_counts ; i++ ) {
                connections.push_back(*(layer_connections_start_ptr + i)) ;
            }
            // Style-Fix: Changed loop boundary to connections.size() to avoid out-of-bounds reads on unpopulated vectors
            for ( int i = 0 ; i < connections.size() ; i++ ) {
                if ( visited.find(connections[i]) == visited.end() ) {
                    visited.insert(connections[i]) ;
                    float new_dist = distance ( query , connections[i] ) ;
                    if ( new_dist < (--result_queue.end())->first || result_queue.size() < ef ) {
                        candidates.insert({new_dist , connections[i]}) ;
                        result_queue.insert({new_dist , connections[i]}) ;
                        if ( result_queue.size() > ef ) {
                            result_queue.erase(--result_queue.end()) ;
                        }
                    }
                }
            }
        }
        return vector < pair < float , int > > (result_queue.begin() , result_queue.end()) ;
    }

    vector < int > get_neighbours_hurestic (vector < float > & query , vector < pair < float , int > > & candidates , int layer , int Mtarget) {
        vector < int > neighbours ;
        for ( auto [dist , cand_id] : candidates ) {
            if ( neighbours.size() >= Mtarget ) {
                break ;
            }
            // if the distace of any of the selected candidates is less than the 
            // query to the given neighbour then we dont need to add it 
            // x -> a -> b 
            // if distace of a to b is less than the distace of x to b then we dont need to add b as a neighbour of x
            // Style-Fix: Unified your variable name to match 'dist_cand_to_query' used in the inner loop
            float dist_cand_to_query = dist ;
            bool add = true ;
            for ( int i = 0 ; i < neighbours.size() ; i++ ) {
                int neighbour_id = neighbours[i] ;
                float dist_neighbour_to_cand = distance ( get_vector(neighbour_id) , get_vector(cand_id) ) ;
                if ( dist_neighbour_to_cand < dist_cand_to_query ) {
                    add = false ; 
                    break ;
                }
            }
            if (add) {
                neighbours.push_back(cand_id) ;
            }
        }
        return neighbours ;
    }

    void addLink (int src , int dest , int layer ) {
        int *links = get_node_connections_start_ptr ( src , layer ) ;
        // need to find the counter of the layer connection of a given node 
        int *nbr_counts = &all_links[src * link_stride + 1 + layer ] ;
        int Mtarget = ( layer == 0 ) ? M0 : M ;
        // Style-Fix: Safely cap insertion to your reserved array bounds to eliminate sequential flat memory overwrites
        if ( *nbr_counts < Mtarget ) {
            links[*nbr_counts] = dest ;
            (*nbr_counts) ++ ;
        }
        // remove links if it exceeds mtarget
    }

    public:
    FlatHSNW(int dim, int m = 16, int ef_c = 64) 
        : dimensions(dim), M(m), M0(m * 2), ef_construction(ef_c), 
          max_layers(16), entry_node(-1), highest_level(-1), curr_node_count(0) {
        
        mL = 1.0f / std::log(M);
        rng.seed(42);
        uniform_dist = uniform_real_distribution<float>(0.0, 1.0);

        // Calculate exact byte stride for the link array
        link_stride = 1 + max_layers + M0 + (max_layers * M);
    }


    void insert (vector < float > &v) {
        int id = curr_node_count ++ ;
        all_embeddings.insert(all_embeddings.end() , v.begin() , v.end()) ;
        all_links.resize(curr_node_count * link_stride , 0) ;

        int target_layer = get_random_layer() ;
        // set the max_layer of the given node 
        *get_node_max_layer_ptr(id) = target_layer ;
        // this node is getting inserted at target_layer
        if ( entry_node == -1 ) {
            entry_node = id ;
            highest_level = target_layer ;
            return ;
        }

        // already the node is inserted 
        // so go to the target layer and do the steps 
        // *** if a node exists in a layer n then it must exist in all the lower level
        //     just like the skip lists 
        int curr_node = entry_node ;

        // no graph construction and altercation till i reach that 
        // layer which the node is gonna get inserted
        for ( int layer = highest_level ; layer > target_layer ; layer -- ) {
            auto search_res = search_layer( v , curr_node , 1 , layer) ;
            if ( !search_res.empty() ) {
                curr_node = search_res[0].second ; // just find the closest
            }
        }

        // now i am at the target layer and i need to insert the node
        int start_layer = min ( target_layer , highest_level ) ;
        for ( int layer = start_layer ; layer >= 0 ; layer -- ) {
            auto candidates = search_layer ( v, curr_node , ef_construction , layer ) ;
            // now i have the candidates closes , but i want to connect nodes such that 
            // i dont form the close cluster rather i want some far connections to 
            // to make the graph navigable and have good coverage 
            // we will use the hurestic to connect the nodes 
            int Mtarget = M ;
            if ( layer == 0 ) {
                Mtarget = M0 ;
            }
            auto neighbours = get_neighbours_hurestic ( v , candidates , layer , Mtarget ) ;
            // we got the neigbhous from the candidates 
            // now we add link 
            for (auto nbr_id : neighbours ) {
                addLink( id , nbr_id , layer) ;
                addLink (nbr_id , id , layer ) ;
                // remove links if it exceeds mtarget
            }
            // descent 
            if ( !candidates.empty() ) {
                curr_node = candidates[0].second ;
            }
        }
        if ( highest_level < target_layer ) {
            entry_node = id;
            highest_level = target_layer ;
        }
    }

    vector < int > search (vector < float > & query , int k , int ef_search ) {
        if ( entry_node == - 1 ) {
            return {};
        }
        int curr_node = entry_node ;
        // fast track to deepest layer 
        for ( int layer = highest_level ; layer > 0 ; layer -- ) {
            auto search_res = search_layer (query , curr_node , 1 , layer) ;
            if ( !search_res.empty() ) {
                curr_node = search_res[0].second ;
            }
        }
        // wide search in this layer 
        auto candidates = search_layer ( query , curr_node , ef_search , 0 ) ;

        while ( candidates.size() > k ) {
            candidates.pop_back() ;
        }
        vector<int> result_ids;
        for (auto &cand : candidates) {
            result_ids.push_back(cand.second); // cand.second is the integer ID of the node
        }
        return result_ids;
    }
    
};


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

int main() {
    int dimensions = 768; 
    FlatHSNW index(dimensions, 4, 16);
    
    // 1. Load the text mappings into memory
    std::vector<std::string> song_names;
    std::ifstream map_file("song_mappings.txt");
    if (!map_file) {
        std::cerr << "WARNING: Could not open song_mappings.txt. Will display raw IDs instead." << std::endl;
    } else {
        std::string line;
        while (std::getline(map_file, line)) {
            if (!line.empty()) song_names.push_back(line);
        }
    }

    // 2. Stream binary embeddings into HNSW
    std::ifstream file("embeddings.bin", std::ios::binary);
    if (!file) {
        std::cerr << "FATAL ERROR: Could not open embeddings.bin." << std::endl;
        return 1;
    }

    std::vector<float> vec(dimensions);
    int count = 0;
    std::cout << "Streaming binary embeddings into HNSW..." << std::endl;

    while (file.read(reinterpret_cast<char*>(vec.data()), dimensions * sizeof(float))) {
        index.insert(vec);
        count++;
    }
    std::cout << "Successfully built graph with " << count << " vectors." << std::endl;

    // 3. Test the search routing
    if (count > 0) {
        // Reset the file pointer to use the 0th vector as our query
        file.clear();
        file.seekg(0, std::ios::beg);
        
        std::vector<float> query_vec(dimensions);
        file.read(reinterpret_cast<char*>(query_vec.data()), dimensions * sizeof(float));

        // Change k to 5 to see the top 5 closest matches across your 556 vectors
        int k = 5;
        int ef_search = 32; 
        
        std::cout << "\nSearching for top " << k << " nearest matches to vector 0..." << std::endl;
        if (!song_names.empty()) {
            std::cout << "Query Target: " << song_names[0] << std::endl;
        }
        std::cout << "------------------------------------------------" << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        
        // Perform search (now returns an array of integer indices)
        std::vector<int> result_ids = index.search(query_vec, k, ef_search);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // 4. Output human-readable matches
        for (size_t i = 0; i < result_ids.size(); ++i) {
            int id = result_ids[i];
            if (id >= 0 && id < static_cast<int>(song_names.size())) {
                std::cout << "Match #" << i + 1 << " (ID: " << id << "): " << song_names[id] << std::endl;
            } else {
                std::cout << "Match #" << i + 1 << " (ID: " << id << "): [No text mapping found]" << std::endl;
            }
        }
        
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Search complete in " << duration.count() << " microseconds." << std::endl;
    }

    return 0;
}
/*
The music streaming service should allow users to browse and search for songs, albums, and artists.
Users should be able to create and manage playlists.
The system should support user authentication and authorization.
Users should be able to play, pause, skip, and seek within songs.
The system should recommend songs and playlists based on user preferences and listening history.
The system should handle concurrent requests and ensure smooth streaming experience for multiple users.
The system should be scalable and handle a large volume of songs and users.
The system should be extensible to support additional features such as social sharing and offline playback

users , songs , albums , aritsts , playlist 

*/