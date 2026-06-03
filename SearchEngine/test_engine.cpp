#include <bits/stdc++.h>
#include "hnsw.h"
using namespace std;

int main() {
    const int dimensions = 3072;
    const string db_path = "embeddings_store.db";

    cout << "=== HNSW Search Engine Verification ===" << endl;

    // 1. Initialize FlatHSNW with the expected dimensions
    FlatHSNW index(dimensions, 4, 16);

    // 2. Load from global store (which dynamically builds the HNSW graph)
    int loaded = index.load_from_store(db_path);
    if (loaded <= 0) {
        cerr << "FATAL ERROR: Failed to load from embedding store or store is empty." << endl;
        return 1;
    }

    // 3. To verify search, load records directly to extract a test query vector.
    int db_dims = 0;
    vector<EmbeddingRecord> records;
    if (!load_database(db_path, db_dims, records)) {
        cerr << "FATAL ERROR: Failed to load database for verification." << endl;
        return 1;
    }

    // 4. Select a random vector from the loaded dataset as a query
    mt19937 rng(12345); // Seeded for reproducibility
    uniform_int_distribution<int> dist(0, loaded - 1);
    int random_idx = dist(rng);

    vector<float> query_vec = records[random_idx].vec;
    string expected_label = records[random_idx].label;

    cout << "\nExecuting HNSW Search..." << endl;
    cout << "Query Vector Target [Index " << random_idx << "]: " << expected_label << endl;
    cout << "--------------------------------------------------------" << endl;

    auto start = chrono::high_resolution_clock::now();
    
    // Execute HNSW search (find top k=5 matches)
    int k = 5;
    int ef_search = 32;
    vector<int> result_ids = index.search(query_vec, k, ef_search);
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

    // 5. Output matches
    bool found_self = false;
    for (size_t i = 0; i < result_ids.size(); ++i) {
        int id = result_ids[i];
        string label = index.get_label(id);
        cout << "Match #" << i + 1 << " (ID: " << id << "): " << label << endl;
        if (label == expected_label) {
            found_self = true;
        }
    }
    
    cout << "--------------------------------------------------------" << endl;
    cout << "Search completed in " << duration.count() << " microseconds." << endl;

    if (found_self) {
        cout << "SUCCESS: Query target successfully retrieved in the search results." << endl;
    } else {
        cout << "WARNING: Query target was NOT retrieved in the search results." << endl;
    }

    return 0;
}
