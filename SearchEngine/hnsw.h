#pragma once
#include <bits/stdc++.h>
#include "embedding_store.h"
using namespace std;

class FlatHSNW {
private :
    int dimensions;
    int M ;
    int M0 ;
    int ef_construction;
    int max_layers ;
    int entry_node ; 
    int highest_level ;
    int curr_node_count ; 
    float mL ;
    mt19937 rng ;
    uniform_real_distribution<float> uniform_dist ;
    
    vector < float > all_embeddings ;
    vector < int > all_links ;
    int link_stride ;

    float* get_vector ( int id ) {
        return &all_embeddings[id * dimensions] ;
    }
    
    int* get_node_max_layer_ptr ( int id ) {
        return &all_links[id * link_stride] ;
    }

    int* get_node_connections_start_ptr ( int id , int layer ) {
        if ( layer > 0 ) {
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
            int * layer_connections_start_ptr = get_node_connections_start_ptr(node_id, layer) ;
            vector < int > connections ;
            int size = M0 ;
            if ( layer > 0 ) {
                size = M ;
            }
            int *nbr_counts = &all_links[node_id * link_stride + 1 + layer ] ;
            for ( int i = 0 ; i < *nbr_counts ; i++ ) {
                connections.push_back(*(layer_connections_start_ptr + i)) ;
            }
            for ( size_t i = 0 ; i < connections.size() ; i++ ) {
                if ( visited.find(connections[i]) == visited.end() ) {
                    visited.insert(connections[i]) ;
                    float new_dist = distance ( query , connections[i] ) ;
                    if ( new_dist < (--result_queue.end())->first || result_queue.size() < static_cast<size_t>(ef) ) {
                        candidates.insert({new_dist , connections[i]}) ;
                        result_queue.insert({new_dist , connections[i]}) ;
                        if ( result_queue.size() > static_cast<size_t>(ef) ) {
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
        for ( auto [dist, cand_id] : candidates ) {
            if ( static_cast<int>(neighbours.size()) >= Mtarget ) {
                break ;
            }
            float dist_cand_to_query = dist ;
            bool add = true ;
            for ( size_t i = 0 ; i < neighbours.size() ; i++ ) {
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
        int *nbr_counts = &all_links[src * link_stride + 1 + layer ] ;
        int Mtarget = ( layer == 0 ) ? M0 : M ;
        if ( *nbr_counts < Mtarget ) {
            links[*nbr_counts] = dest ;
            (*nbr_counts) ++ ;
        }
    }

public:
    vector<string> labels;

    FlatHSNW(int dim, int m = 16, int ef_c = 64) 
        : dimensions(dim), M(m), M0(m * 2), ef_construction(ef_c), 
          max_layers(16), entry_node(-1), highest_level(-1), curr_node_count(0) {
        
        mL = 1.0f / log(M);
        rng.seed(42);
        uniform_dist = uniform_real_distribution<float>(0.0, 1.0);
        link_stride = 1 + max_layers + M0 + (max_layers * M);
    }

    void insert (vector < float > &v) {
        int id = curr_node_count ++ ;
        all_embeddings.insert(all_embeddings.end() , v.begin() , v.end()) ;
        all_links.resize(curr_node_count * link_stride , 0) ;

        int target_layer = get_random_layer() ;
        *get_node_max_layer_ptr(id) = target_layer ;
        if ( entry_node == -1 ) {
            entry_node = id ;
            highest_level = target_layer ;
            return ;
        }

        int curr_node = entry_node ;

        for ( int layer = highest_level ; layer > target_layer ; layer -- ) {
            auto search_res = search_layer( v , curr_node , 1 , layer) ;
            if ( !search_res.empty() ) {
                curr_node = search_res[0].second ;
            }
        }

        int start_layer = min ( target_layer , highest_level ) ;
        for ( int layer = start_layer ; layer >= 0 ; layer -- ) {
            auto candidates = search_layer ( v, curr_node , ef_construction , layer ) ;
            int Mtarget = M ;
            if ( layer == 0 ) {
                Mtarget = M0 ;
            }
            auto neighbours = get_neighbours_hurestic ( v , candidates , layer , Mtarget ) ;
            for (auto nbr_id : neighbours ) {
                addLink( id , nbr_id , layer) ;
                addLink (nbr_id , id , layer ) ;
            }
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
        for ( int layer = highest_level ; layer > 0 ; layer -- ) {
            auto search_res = search_layer (query , curr_node , 1 , layer) ;
            if ( !search_res.empty() ) {
                curr_node = search_res[0].second ;
            }
        }
        auto candidates = search_layer ( query , curr_node , ef_search , 0 ) ;

        while ( static_cast<int>(candidates.size()) > k ) {
            candidates.pop_back() ;
        }
        vector<int> result_ids;
        for (auto &cand : candidates) {
            result_ids.push_back(cand.second);
        }
        return result_ids;
    }

    string get_label(int id) const {
        if (id >= 0 && id < static_cast<int>(labels.size())) {
            return labels[id];
        }
        return "[Unknown ID]";
    }

    int load_from_store(const string& db_path, float duplicate_epsilon = 1e-6f) {
        int db_dims = 0;
        vector<EmbeddingRecord> records;
        if (!load_database(db_path, db_dims, records, duplicate_epsilon)) {
            cerr << "WARNING: Could not load embeddings store from " << db_path 
                 << ". Graph remains empty." << endl;
            return 0;
        }

        if (db_dims != dimensions) {
            cerr << "ERROR: Database embedding dimensions (" << db_dims 
                 << ") do not match search engine configuration (" << dimensions << ")" << endl;
            return -1;
        }

        entry_node = -1;
        highest_level = -1;
        curr_node_count = 0;
        all_embeddings.clear();
        all_links.clear();
        labels.clear();

        cout << "Building HNSW index from global store (" << records.size() << " records)..." << endl;
        for (auto& rec : records) {
            labels.push_back(rec.label);
            insert(rec.vec);
        }
        cout << "Successfully built HNSW graph with " << curr_node_count << " nodes." << endl;
        return curr_node_count;
    }
};
