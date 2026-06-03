#include <bits/stdc++.h>
#include "embedding_store.h"
using namespace std;

void print_usage() {
    cout << "Usage: insert_tool.exe <db_path> <dimensions> <label> <vector_source> [--binary]" << endl;
    cout << "where <vector_source> can be:" << endl;
    cout << "  --stdin                        Read vector from standard input (whitespace or comma separated floats)" << endl;
    cout << "  <file_path>                    Read vector from a text file (whitespace or comma separated floats)" << endl;
    cout << "  <binary_file_path> --binary    Read vector as raw binary floats from a file" << endl;
    cout << "  <comma_separated_floats>       Direct input of comma-separated floats (e.g. 0.1,0.2,0.3)" << endl;
}

vector<float> parse_text_floats(istream& in, int expected_size) {
    vector<float> vec;
    string token;
    while (in >> token) {
        stringstream ss(token);
        string subtoken;
        while (getline(ss, subtoken, ',')) {
            if (subtoken.empty()) continue;
            try {
                vec.push_back(stof(subtoken));
            } catch (...) {
                // Ignore invalid tokens
            }
        }
        if (static_cast<int>(vec.size()) >= expected_size) {
            break;
        }
    }
    return vec;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        print_usage();
        return 1;
    }

    string db_path = argv[1];
    int dimensions = 0;
    try {
        dimensions = stoi(argv[2]);
    } catch (...) {
        cerr << "ERROR: Dimensions must be a valid integer." << endl;
        return 1;
    }

    if (dimensions <= 0) {
        cerr << "ERROR: Dimensions must be greater than 0." << endl;
        return 1;
    }

    string label = argv[3];
    string source = argv[4];
    bool is_binary = (argc >= 6 && string(argv[5]) == "--binary");

    vector<float> vector;

    if (source == "--stdin") {
        cout << "Reading " << dimensions << " floats from stdin..." << endl;
        vector = parse_text_floats(cin, dimensions);
    } else if (is_binary) {
        ifstream file(source, ios::binary);
        if (!file) {
            cerr << "ERROR: Cannot open binary file: " << source << endl;
            return 1;
        }
        vector.resize(dimensions);
        if (!file.read(reinterpret_cast<char*>(vector.data()), dimensions * sizeof(float))) {
            cerr << "ERROR: Failed to read " << dimensions << " floats from binary file." << endl;
            return 1;
        }
    } else {
        ifstream file(source);
        if (file) {
            cout << "Reading " << dimensions << " floats from text file: " << source << endl;
            vector = parse_text_floats(file, dimensions);
        } else {
            stringstream ss(source);
            vector = parse_text_floats(ss, dimensions);
        }
    }

    if (static_cast<int>(vector.size()) < dimensions) {
        cerr << "ERROR: Read only " << vector.size() << " floats, but dimensions required is " << dimensions << "." << endl;
        return 1;
    }
    if (static_cast<int>(vector.size()) > dimensions) {
        vector.resize(dimensions);
    }

    bool inserted = insert_to_store(db_path, dimensions, label, vector, 1e-6f);
    if (inserted) {
        return 0;
    } else {
        return 2;
    }
}
