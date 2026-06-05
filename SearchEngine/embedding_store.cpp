#include "embedding_store.h"
#include <fstream>
#include <iostream>

// ============================================================================
// FILE READING / DESERIALIZATION
// ============================================================================
bool load_database(const std::string& path, int& dimensions, std::vector<EmbeddingRecord>& records, float duplicate_epsilon) {
    // SYNTAX: std::ios::binary forces the OS to treat the file as raw bytes.
    // AIM: Prevents automated newline translation (\n <-> \r\n) from altering and corrupting numeric data.
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false; 
    }

    // AIM: Read and verify the hardcoded 4-byte file signature ("magic bytes").
    // If the file does not start with these exact bytes, it is either corrupt or not our format.
    char magic[4];
    if (!file.read(magic, 4)) return false;
    if (magic[0] != 'E' || magic[1] != 'M' || magic[2] != 'B' || magic[3] != 'D') {
        std::cerr << "ERROR: Invalid database file format." << std::endl;
        return false;
    }

    // SYNTAX: std::int32_t explicitly forces a 4-byte size, ensuring cross-platform file compatibility.
    std::int32_t dims = 0;
    std::int32_t num_vectors = 0;
    
    // SYNTAX: reinterpret_cast<char*> casts the address of our integer into a raw byte pointer.
    // SYNTAX: sizeof(dims) tells the stream exactly how many bytes (4) to pull from the disk.
    // AIM: Blit the raw bytes from the disk directly into the memory location of the variable.
    if (!file.read(reinterpret_cast<char*>(&dims), sizeof(dims))) return false;
    if (!file.read(reinterpret_cast<char*>(&num_vectors), sizeof(num_vectors))) return false;

    dimensions = dims;
    records.clear(); 

    

    // AIM: Sequentially read the continuous stream of structured records from the disk.
    for (std::int32_t i = 0; i < num_vectors; ++i) {
        std::int32_t label_len = 0;
        // Read the 4-byte integer indicating the length of the upcoming string
        if (!file.read(reinterpret_cast<char*>(&label_len), sizeof(label_len))) break;

        // AIM: Prepare a contiguous memory buffer inside the string to receive disk bytes.
        std::string label(label_len, '\0');
        // SYNTAX: &label[0] passes the raw pointer to the start of the string's internal memory block.
        if (!file.read(&label[0], label_len)) break;

        // AIM: Prepare a contiguous memory buffer inside the vector for the float array.
        std::vector<float> vec(dims);
        // SYNTAX: vec.data() fetches the raw memory address of the first float element.
        // AIM: Read the entire high-dimensional vector from disk into memory in a single I/O operation.
        if (!file.read(reinterpret_cast<char*>(vec.data()), dims * sizeof(float))) break;

        // [Deduplication logic happens here in memory]
        records.push_back({label, vec});
    }

    return true;
}

// ============================================================================
// FILE WRITING / SERIALIZATION
// ============================================================================
bool save_database(const std::string& path, int dimensions, const std::vector<EmbeddingRecord>& records) {
    // SYNTAX: std::ios::trunc instantly chops the existing file size down to 0 bytes.
    // AIM: Wipes old file contents so no trailing garbage data remains if the new DB is smaller.
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cerr << "ERROR: Failed to open file for writing: " << path << std::endl;
        return false;
    }

    // AIM: Write the global 4-byte file identifier to the very front of the stream.
    char magic[4] = {'E', 'M', 'B', 'D'};
    file.write(magic, 4);

    std::int32_t dims = dimensions;
    std::int32_t num_vectors = static_cast<std::int32_t>(records.size());
    
    // SYNTAX: reinterpret_cast<const char*> treats the integer memory addresses as read-only byte streams.
    // AIM: Stream the raw metadata bytes directly onto the physical storage media.
    file.write(reinterpret_cast<const char*>(&dims), sizeof(dims));
    file.write(reinterpret_cast<const char*>(&num_vectors), sizeof(num_vectors));

    // AIM: Loop through memory arrays and flatten them sequentially out to the file.
    for (const auto& rec : records) {
        std::int32_t label_len = static_cast<std::int32_t>(rec.label.size());
        
        // Write string length descriptor (4 bytes)
        file.write(reinterpret_cast<const char*>(&label_len), sizeof(label_len));
        // SYNTAX: rec.label.data() exposes the raw character array pointer.
        // AIM: Write string characters to disk without any null-terminators or delimiters.
        file.write(rec.label.data(), label_len);
        // SYNTAX: rec.vec.data() exposes the raw float array pointer.
        // AIM: Write the entire block of high-dimensional floats sequentially into the file.
        file.write(reinterpret_cast<const char*>(rec.vec.data()), dimensions * sizeof(float));
    }

    // SYNTAX: file.good() checks the internal bit flags of the stream system.
    // AIM: Verifies if the physical write operations actually succeeded (returns false if disk is full).
    return file.good(); 
}