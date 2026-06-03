#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <cmath>

// Struct to represent a single database entry.
// Combines a textual identifier with its high-dimensional float vector.
struct EmbeddingRecord {
    std::string label;
    std::vector<float> vec;
};

// Check if two vectors are duplicates within a geometric epsilon tolerance.
inline bool is_duplicate_vector(const std::vector<float>& v1, const std::vector<float>& v2, float epsilon = 1e-6f) {
    if (v1.size() != v2.size()) return false;
    float dist_sq = 0.0f;
    for (size_t i = 0; i < v1.size(); ++i) {
        float diff = v1[i] - v2[i];
        dist_sq += diff * diff;
    }
    float dist = std::sqrt(dist_sq);
    return dist < epsilon;
}

// Load all records from a binary file. Removes any duplicate records found during load.
inline bool load_database(const std::string& path, int& dimensions, std::vector<EmbeddingRecord>& records, float duplicate_epsilon = 1e-6f) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false; 
    }

    char magic[4];
    if (!file.read(magic, 4)) return false;
    if (magic[0] != 'E' || magic[1] != 'M' || magic[2] != 'B' || magic[3] != 'D') {
        std::cerr << "ERROR: Invalid database file format." << std::endl;
        return false;
    }

    std::int32_t dims = 0;
    std::int32_t num_vectors = 0;
    
    if (!file.read(reinterpret_cast<char*>(&dims), sizeof(dims))) return false;
    if (!file.read(reinterpret_cast<char*>(&num_vectors), sizeof(num_vectors))) return false;

    dimensions = dims;
    records.clear(); 

    for (std::int32_t i = 0; i < num_vectors; ++i) {
        std::int32_t label_len = 0;
        if (!file.read(reinterpret_cast<char*>(&label_len), sizeof(label_len))) break;

        std::string label(label_len, '\0');
        if (!file.read(&label[0], label_len)) break;

        std::vector<float> vec(dims);
        if (!file.read(reinterpret_cast<char*>(vec.data()), dims * sizeof(float))) break;

        records.push_back({label, vec});
    }

    return true;
}

// Overwrites or creates a db file, serializing the records to disk in a tight binary format.
inline bool save_database(const std::string& path, int dimensions, const std::vector<EmbeddingRecord>& records) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cerr << "ERROR: Failed to open file for writing: " << path << std::endl;
        return false;
    }

    char magic[4] = {'E', 'M', 'B', 'D'};
    file.write(magic, 4);

    std::int32_t dims = dimensions;
    std::int32_t num_vectors = static_cast<std::int32_t>(records.size());
    
    file.write(reinterpret_cast<const char*>(&dims), sizeof(dims));
    file.write(reinterpret_cast<const char*>(&num_vectors), sizeof(num_vectors));

    for (const auto& rec : records) {
        std::int32_t label_len = static_cast<std::int32_t>(rec.label.size());
        
        file.write(reinterpret_cast<const char*>(&label_len), sizeof(label_len));
        file.write(rec.label.data(), label_len);
        file.write(reinterpret_cast<const char*>(rec.vec.data()), dimensions * sizeof(float));
    }

    return file.good(); 
}

// High-level API: Loads database, scans for duplicates, appends if unique, and rewrites the file.
inline bool insert_to_store(const std::string& path, int dimensions, const std::string& label, const std::vector<float>& vec, float duplicate_epsilon = 1e-6f) {
    if (static_cast<int>(vec.size()) != dimensions) {
        std::cerr << "ERROR: Vector dimensions mismatch. Expected " << dimensions 
             << ", got " << vec.size() << std::endl;
        return false;
    }

    int existing_dims = dimensions;
    std::vector<EmbeddingRecord> records;
    
    bool exists = load_database(path, existing_dims, records, duplicate_epsilon);
    if (exists && existing_dims != dimensions) {
        std::cerr << "ERROR: Database dimension mismatch. DB has " << existing_dims 
             << ", attempting to insert vector of dimension " << dimensions << std::endl;
        return false;
    }

    for (const auto& rec : records) {
        if (is_duplicate_vector(rec.vec, vec, duplicate_epsilon)) {
            std::cout << "[DEDUPLICATOR] Vector for label '" << label 
                 << "' matches existing vector for label '" << rec.label 
                 << "'. Skipping insertion." << std::endl;
            return false;
        }
    }

    records.push_back({label, vec});

    if (save_database(path, dimensions, records)) {
        std::cout << "[STORE] Successfully inserted new embedding for '" << label << "'." << std::endl;
        return true;
    } else {
        std::cerr << "[STORE] Failed to save database file." << std::endl;
        return false;
    }
}