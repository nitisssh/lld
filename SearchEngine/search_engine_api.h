#pragma once

/*
 * ============================================================================
 * HEADER-ONLY SEARCH ENGINE ARCHITECTURE UNDER THE HOOD
 * ============================================================================
 * Why is this setup header-only? How does it fix the "undefined reference" issue?
 * 
 * 1. The Old Way: Promise vs. Delivery
 *    Normally in C++, code is split into two parts:
 *    - Header file (.h): The "promise" telling the compiler that a function exists.
 *    - Source file (.cpp): The "delivery" containing the actual function code.
 *    If you only compile `Spotify.cpp` with `g++ Spotify.cpp`, the compiler accepts 
 *    the promises, but the Linker throws an "undefined reference" error because
 *    nobody compiled `search_engine_api.cpp` to deliver the actual code.
 * 
 * 2. The New Way: Header-Only
 *    We refactored the search engine into a header-only library by moving all the 
 *    definitions (implementations) from the .cpp files directly into the .h files.
 *    Now, when you #include "search_engine_api.h", the preprocessor copies the entire 
 *    implementation code straight into your file before compiling. This allows a 
 *    single compilation command (e.g., `g++ Spotify.cpp`) to build everything.
 * 
 * 3. The Magic Keywords
 *    A. The 'inline' keyword (for functions):
 *       Tells the compiler that if multiple files include this header, it's 
 *       fine to compile the functions multiple times—just merge them into one
 *       definition during linking to avoid "multiple definition" errors.
 *    B. C++17 'inline' variables (for global state):
 *       Our search engine has global variables (like `global_index` for the graph).
 *       Without `inline`, every file including this header would get a separate copy
 *       of the variables in memory. Using `inline` guarantees that they all share
 *       the exact same single instance in memory across the entire program.
 * ============================================================================
 */

#include <bits/stdc++.h>
#include "hnsw.h"
#include "embedding_store.h"
using namespace std;

namespace HNSWEngine {
    inline unique_ptr<FlatHSNW> global_index = nullptr;
    inline string resolved_db_path = "";
    inline int configured_dimensions = 0;

    // 1. Initialize engine from a path (file or directory). Builds the HNSW graph dynamically.
    // If a directory is provided, it resolves to path/embeddings_store.db.
    // Returns true on success, false on failure.
    inline bool Initialize(const string& path, int dimensions, int M = 16, int ef_construction = 64) {
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

    // 1.5. Initialize engine from legacy raw binary embeddings and a mapping text file.
    // Consolidates them, removes duplicates to build a unique database, saves it to output_db_path,
    // and then initializes the engine using that database file.
    inline bool InitializeFromLegacy(const string& bin_path, const string& mapping_path, const string& output_db_path, int dimensions=768, int M = 16, int ef_construction = 64, float duplicate_epsilon = 1e-6f) {
        cout << "[API] Migrating legacy files: " << bin_path << " and " << mapping_path << " to " << output_db_path << endl;

        // Load textual mappings
        vector<string> labels;
        ifstream map_file(mapping_path);
        if (!map_file) {
            cerr << "[API ERROR] Missing mapping file: " << mapping_path << endl;
            return false;
        }
        string line;
        while (getline(map_file, line)) {
            if (!line.empty()) {
                labels.push_back(line);
            }
        }

        // Load legacy raw binary embeddings
        ifstream bin_file(bin_path, ios::binary);
        if (!bin_file) {
            cerr << "[API ERROR] Missing binary embeddings file: " << bin_path << endl;
            return false;
        }

        vector<vector<float>> database_vectors;
        vector<float> vec(dimensions);
        int count = 0;
        while (bin_file.read(reinterpret_cast<char*>(vec.data()), dimensions * sizeof(float))) {
            database_vectors.push_back(vec);
            count++;
        }

        int min_size = min(count, static_cast<int>(labels.size()));

        // Deduplicate and pair them in memory
        vector<EmbeddingRecord> unique_records;
        int duplicate_count = 0;
        for (int i = 0; i < min_size; ++i) {
            bool dup = false;
            for (const auto& existing : unique_records) {
                if (is_duplicate_vector(existing.vec, database_vectors[i], duplicate_epsilon)) {
                    dup = true;
                    break;
                }
            }

            if (!dup) {
                unique_records.push_back({labels[i], database_vectors[i]});
            } else {
                duplicate_count++;
            }
        }

        cout << "[API] Loaded " << count << " raw embeddings and " << labels.size() << " labels." << endl;
        cout << "[API] Deduplicated " << duplicate_count << " vectors. " << unique_records.size() << " unique records remaining." << endl;

        // Save to consolidated DB file
        if (!save_database(output_db_path, dimensions, unique_records)) {
            cerr << "[API ERROR] Failed to save consolidated database to: " << output_db_path << endl;
            return false;
        }

        // Initialize using the newly created DB file
        return Initialize(output_db_path, dimensions, M, ef_construction);
    }

    // 2. Insert a new embedding vector (checking for duplicates). Writes to DB and active graph.
    // Returns true on success, false if duplicate or write failure.
    inline bool Insert(const string& label, const vector<float>& vec, float duplicate_epsilon = 1e-6f) {
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

    // 3. Search for top k nearest neighbors. Returns the labels of the matches.
    inline vector<string> Search(const vector<float>& query, int k, int ef_search) {
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

    // Expose the count of loaded nodes in the active index.
    inline int GetNodeCount() {
        if (!global_index) return 0;
        return static_cast<int>(global_index->labels.size());
    }
}
