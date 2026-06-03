#include <bits/stdc++.h>
#include "embedding_store.h"
using namespace std;

int main() {
    const int dimensions = 3072;
    const string song_map_path = "song_mappings.txt";
    const string embed_bin_path = "embeddings.bin";
    const string db_path = "embeddings_store.db";

    cout << "Starting migration from legacy files..." << endl;

    // 1. Load song textual mappings
    vector<string> song_names;
    ifstream map_file(song_map_path);
    if (!map_file) {
        cerr << "FATAL ERROR: Missing " << song_map_path << endl;
        return 1;
    }
    string line;
    while (getline(map_file, line)) {
        if (!line.empty()) {
            song_names.push_back(line);
        }
    }
    cout << "Loaded " << song_names.size() << " song names." << endl;

    // 2. Load legacy raw binary embeddings
    ifstream db_file(embed_bin_path, ios::binary);
    if (!db_file) {
        cerr << "FATAL ERROR: Missing " << embed_bin_path << endl;
        return 1;
    }

    vector<vector<float>> database_vectors;
    vector<float> vec(dimensions);
    int count = 0;

    while (db_file.read(reinterpret_cast<char*>(vec.data()), dimensions * sizeof(float))) {
        database_vectors.push_back(vec);
        count++;
    }
    cout << "Loaded " << count << " raw embeddings from " << embed_bin_path << endl;

    if (count != static_cast<int>(song_names.size())) {
        cerr << "WARNING: Mismatch! Binary vectors: " << count 
             << " | Song names: " << song_names.size() << endl;
    }

    int min_size = min(count, static_cast<int>(song_names.size()));

    // 3. Deduplicate and pair them in memory
    vector<EmbeddingRecord> unique_records;
    int duplicate_count = 0;

    for (int i = 0; i < min_size; ++i) {
        bool dup = false;
        for (const auto& existing : unique_records) {
            if (is_duplicate_vector(existing.vec, database_vectors[i], 1e-6f)) {
                dup = true;
                break;
            }
        }

        if (!dup) {
            unique_records.push_back({song_names[i], database_vectors[i]});
        } else {
            duplicate_count++;
            cout << "[MIGRATOR] Skipping duplicate vector associated with: " << song_names[i] << endl;
        }
    }

    cout << "Found and removed " << duplicate_count << " duplicate records." << endl;

    // 4. Save to the new consolidated database
    if (save_database(db_path, dimensions, unique_records)) {
        cout << "SUCCESS: Saved " << unique_records.size() 
             << " unique records to " << db_path << endl;
    } else {
        cerr << "ERROR: Failed to save to " << db_path << endl;
        return 1;
    }

    return 0;
}
