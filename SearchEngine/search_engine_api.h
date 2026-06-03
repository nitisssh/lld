#pragma once
#include <bits/stdc++.h>
using namespace std;

namespace HNSWEngine {
    // 1. Initialize engine from a path (file or directory). Builds the HNSW graph dynamically.
    // If a directory is provided, it resolves to path/embeddings_store.db.
    // Returns true on success, false on failure.
    bool Initialize(const string& path, int dimensions, int M = 16, int ef_construction = 64);

    // 2. Insert a new embedding vector (checking for duplicates). Writes to DB and active graph.
    // Returns true on success, false if duplicate or write failure.
    bool Insert(const string& label, const vector<float>& vec, float duplicate_epsilon = 1e-6f);

    // 3. Search for top k nearest neighbors. Returns the labels of the matches.
    vector<string> Search(const vector<float>& query, int k, int ef_search);

    // Expose the count of loaded nodes in the active index.
    int GetNodeCount();
}
