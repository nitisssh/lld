#include <bits/stdc++.h>
#include "search_engine_api.h"
#include "hnsw.h"
#include "embedding_store.h"
using namespace std;

namespace HNSWEngine {
    namespace {
        unique_ptr<FlatHSNW> global_index = nullptr;
        string resolved_db_path = "";
        int configured_dimensions = 0;
    }

    bool Initialize(const string& path, int dimensions, int M, int ef_construction) {
        string target_path = path;
        try {
            if (filesystem::is_directory(path)) {
                target_path = (filesystem::path(path) / "embeddings_store.db").string();
            }
        } catch (...) {
            // Fallback if filesystem checks encounter issues
        }

        resolved_db_path = target_path;
        configured_dimensions = dimensions;

        global_index = make_unique<FlatHSNW>(dimensions, M, ef_construction);

        int loaded = global_index->load_from_store(resolved_db_path);
        if (loaded < 0) {
            cerr << "[API] Failed to initialize engine: dimension mismatch or database load error." << endl;
            global_index.reset();
            return false;
        }

        cout << "[API] Engine initialized successfully. Active graph built with " 
             << loaded << " nodes from: " << resolved_db_path << endl;
        return true;
    }

    bool Insert(const string& label, const vector<float>& vec, float duplicate_epsilon) {
        if (!global_index) {
            cerr << "[API ERROR] Engine not initialized. Call Initialize() first." << endl;
            return false;
        }

        if (static_cast<int>(vec.size()) != configured_dimensions) {
            cerr << "[API ERROR] Incompatible vector dimensions. Expected " 
                 << configured_dimensions << ", got " << vec.size() << endl;
            return false;
        }

        bool success = insert_to_store(resolved_db_path, configured_dimensions, label, vec, duplicate_epsilon);
        if (!success) {
            return false;
        }

        global_index->labels.push_back(label);
        
        vector<float> vec_copy = vec;
        global_index->insert(vec_copy);

        return true;
    }

    vector<string> Search(const vector<float>& query, int k, int ef_search) {
        if (!global_index) {
            cerr << "[API ERROR] Engine not initialized. Call Initialize() first." << endl;
            return {};
        }

        vector<float> query_copy = query;
        vector<int> match_ids = global_index->search(query_copy, k, ef_search);

        vector<string> match_labels;
        for (int id : match_ids) {
            match_labels.push_back(global_index->get_label(id));
        }

        return match_labels;
    }

    int GetNodeCount() {
        if (!global_index) return 0;
        return static_cast<int>(global_index->labels.size());
    }
}
