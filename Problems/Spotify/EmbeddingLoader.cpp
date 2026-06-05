#include "EmbeddingLoader.h"
#include "YourHNSWHeader.h" 
#include <fstream>
#include <iostream>

int LoadBinaryEmbeddings(const std::string& filename, FlatHSNW& index) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "ERROR: Code 404. Cannot open embedding file: " << filename << std::endl;
        return -1;
    }

    const int dimensions = 768; // Gemini embedding fixed dimension size
    std::vector<float> vec(dimensions);
    int count = 0;

    // Direct memory streaming loop
    while (file.read(reinterpret_cast<char*>(vec.data()), dimensions * sizeof(float))) {
        index.insert(vec);
        count++;
    }

    return count;
}