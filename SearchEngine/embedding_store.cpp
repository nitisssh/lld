#include <bits/stdc++.h>
#include "embedding_store.h"
using namespace std;

bool is_duplicate_vector(const vector<float>& v1, const vector<float>& v2, float epsilon) {
    if (v1.size() != v2.size()) return false;
    float dist_sq = 0.0f;
    for (size_t i = 0; i < v1.size(); ++i) {
        float diff = v1[i] - v2[i];
        dist_sq += diff * diff;
    }
    float dist = sqrt(dist_sq);
    return dist < epsilon;
}

bool load_database(const string& path, int& dimensions, vector<EmbeddingRecord>& records, float duplicate_epsilon) {
    ifstream file(path, ios::binary);
    if (!file) {
        return false;
    }

    char magic[4];
    if (!file.read(magic, 4)) return false;
    if (magic[0] != 'E' || magic[1] != 'M' || magic[2] != 'B' || magic[3] != 'D') {
        cerr << "ERROR: Invalid database file format (incorrect magic bytes)." << endl;
        return false;
    }

    int32_t dims = 0;
    int32_t num_vectors = 0;
    if (!file.read(reinterpret_cast<char*>(&dims), sizeof(dims))) return false;
    if (!file.read(reinterpret_cast<char*>(&num_vectors), sizeof(num_vectors))) return false;

    dimensions = dims;
    records.clear();

    for (int32_t i = 0; i < num_vectors; ++i) {
        int32_t label_len = 0;
        if (!file.read(reinterpret_cast<char*>(&label_len), sizeof(label_len))) break;

        string label(label_len, '\0');
        if (!file.read(&label[0], label_len)) break;

        vector<float> vec(dims);
        if (!file.read(reinterpret_cast<char*>(vec.data()), dims * sizeof(float))) break;

        // Check for duplicates before adding
        bool dup = false;
        for (const auto& existing : records) {
            if (is_duplicate_vector(existing.vec, vec, duplicate_epsilon)) {
                dup = true;
                break;
            }
        }

        if (!dup) {
            records.push_back({label, vec});
        } else {
            cout << "[DEDUPLICATOR] Removed duplicate embedding for: " << label << endl;
        }
    }

    return true;
}

bool save_database(const string& path, int dimensions, const vector<EmbeddingRecord>& records) {
    ofstream file(path, ios::binary | ios::trunc);
    if (!file) {
        cerr << "ERROR: Failed to open file for writing: " << path << endl;
        return false;
    }

    char magic[4] = {'E', 'M', 'B', 'D'};
    file.write(magic, 4);

    int32_t dims = dimensions;
    int32_t num_vectors = static_cast<int32_t>(records.size());
    file.write(reinterpret_cast<const char*>(&dims), sizeof(dims));
    file.write(reinterpret_cast<const char*>(&num_vectors), sizeof(num_vectors));

    for (const auto& rec : records) {
        int32_t label_len = static_cast<int32_t>(rec.label.size());
        file.write(reinterpret_cast<const char*>(&label_len), sizeof(label_len));
        file.write(rec.label.data(), label_len);
        file.write(reinterpret_cast<const char*>(rec.vec.data()), dimensions * sizeof(float));
    }

    return file.good();
}

bool insert_to_store(const string& path, int dimensions, const string& label, const vector<float>& vec, float duplicate_epsilon) {
    if (static_cast<int>(vec.size()) != dimensions) {
        cerr << "ERROR: Vector dimensions mismatch. Expected " << dimensions 
             << ", got " << vec.size() << endl;
        return false;
    }

    int existing_dims = dimensions;
    vector<EmbeddingRecord> records;
    
    bool exists = load_database(path, existing_dims, records, duplicate_epsilon);
    if (exists && existing_dims != dimensions) {
        cerr << "ERROR: Database dimension mismatch. DB has " << existing_dims 
             << ", attempting to insert vector of dimension " << dimensions << endl;
        return false;
    }

    for (const auto& rec : records) {
        if (is_duplicate_vector(rec.vec, vec, duplicate_epsilon)) {
            cout << "[DEDUPLICATOR] Vector for label '" << label 
                 << "' matches existing vector for label '" << rec.label 
                 << "'. Skipping insertion." << endl;
            return false;
        }
    }

    records.push_back({label, vec});

    if (save_database(path, dimensions, records)) {
        cout << "[STORE] Successfully inserted new embedding for '" << label << "'." << endl;
        return true;
    } else {
        cerr << "[STORE] Failed to save database file." << endl;
        return false;
    }
}
