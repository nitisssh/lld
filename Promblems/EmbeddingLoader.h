#pragma once
#include <string>
#include <vector>

// Forward declaration of your HNSW class so you don't have to include 
// the massive HNSW header here. This keeps compilation fast.
class FlatHSNW; 

/**
 * Streams a raw 768-dimensional float32 binary file directly into an HNSW index.
 * @param filename Path to the .bin file (e.g., "embeddings.bin")
 * @param index Reference to your initialized HNSW index object
 * @return The total number of vectors successfully loaded
 */
int LoadBinaryEmbeddings(const std::string& filename, FlatHSNW& index);