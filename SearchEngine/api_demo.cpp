#include <bits/stdc++.h>
#include "search_engine_api.h"
using namespace std;

int main() {
    const int dimensions = 3072;
    const string dir_path = ".";

    cout << "=== HNSW Search Engine High-Level API Demo ===" << endl;

    // 1. Initialize engine
    if (!HNSWEngine::Initialize(dir_path, dimensions, 4, 16)) {
        cerr << "Initialization failed!" << endl;
        return 1;
    }

    cout << "Engine successfully initialized. Total nodes in graph: " 
         << HNSWEngine::GetNodeCount() << endl;

    // Generate a dummy query vector (let's use 3072 values of 0.1)
    vector<float> query_vec(dimensions, 0.1f);

    // 2. Perform a search
    cout << "\nExecuting Search for dummy query vector..." << endl;
    vector<string> results = HNSWEngine::Search(query_vec, 3, 32);
    
    cout << "Top Matches:" << endl;
    for (size_t i = 0; i < results.size(); ++i) {
        cout << "  Match #" << i + 1 << ": " << results[i] << endl;
    }

    // 3. Attempt to insert a new unique vector
    vector<float> new_vec(dimensions, 0.77f);
    string new_label = "[Pop] Custom API Track";

    cout << "\nInserting new unique vector: '" << new_label << "'" << endl;
    bool inserted1 = HNSWEngine::Insert(new_label, new_vec);
    if (inserted1) {
        cout << "SUCCESS: Inserted successfully. Active nodes now: " 
             << HNSWEngine::GetNodeCount() << endl;
    } else {
        cerr << "ERROR: Failed to insert vector." << endl;
    }

    // 4. Attempt to insert a duplicate of the vector we just added
    cout << "\nInserting duplicate vector (should be rejected):" << endl;
    bool inserted2 = HNSWEngine::Insert("[Pop] Duplicate API Track", new_vec);
    if (!inserted2) {
        cout << "SUCCESS: Duplicate correctly rejected. Active nodes remains: " 
             << HNSWEngine::GetNodeCount() << endl;
    } else {
        cerr << "WARNING: Duplicate was unexpectedly accepted!" << endl;
    }

    // 5. Query for the inserted vector to make sure it exists in the active graph
    cout << "\nSearching for the newly inserted vector (using itself as query)..." << endl;
    vector<string> search_added = HNSWEngine::Search(new_vec, 1, 32);
    if (!search_added.empty()) {
        cout << "Top Match returned: " << search_added[0] << endl;
        if (search_added[0] == new_label) {
            cout << "SUCCESS: The dynamic active index search is working correctly!" << endl;
        }
    }

    return 0;
}
