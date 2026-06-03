#pragma once
#include <bits/stdc++.h>
using namespace std;

struct EmbeddingRecord {
    string label;
    vector<float> vec;
};

// Check if two vectors are duplicates within an epsilon tolerance
bool is_duplicate_vector(const vector<float>& v1, const vector<float>& v2, float epsilon = 1e-6f);

// Load all records from a db file. Removes any duplicate records found during load.
bool load_database(const string& path, int& dimensions, vector<EmbeddingRecord>& records, float duplicate_epsilon = 1e-6f);

// Save all records to a db file.
bool save_database(const string& path, int dimensions, const vector<EmbeddingRecord>& records);

// Insert a new record into the db file if it is not a duplicate.
// If the database file does not exist, it creates a new one.
bool insert_to_store(const string& path, int dimensions, const string& label, const vector<float>& vec, float duplicate_epsilon = 1e-6f);
