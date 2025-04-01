#include "pim_compiler.h"
#include <regex>
#include <fstream>
#include <iostream>
#include <string>

MatrixDimensions parseMatrixMultiply(const std::string& filename) {
    MatrixDimensions dims;
    dims.M = 64;  // Default values
    dims.N = 64;
    dims.K = 64;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file " << filename << ". Using default matrix dimensions." << std::endl;
        return dims;
    }
    
    std::string line;
    std::regex m_pattern("#define\\s+M\\s+(\\d+)");
    std::regex n_pattern("#define\\s+N\\s+(\\d+)");
    std::regex k_pattern("#define\\s+K\\s+(\\d+)");
    
    std::smatch match;
    
    while (std::getline(file, line)) {
        if (std::regex_search(line, match, m_pattern) && match.size() > 1) {
            dims.M = std::stoi(match[1]);
        } 
        else if (std::regex_search(line, match, n_pattern) && match.size() > 1) {
            dims.N = std::stoi(match[1]);
        }
        else if (std::regex_search(line, match, k_pattern) && match.size() > 1) {
            dims.K = std::stoi(match[1]);
        }
    }
    
    std::cout << "Parsed matrix dimensions: " << dims.M << "x" << dims.K << " * " 
              << dims.K << "x" << dims.N << std::endl;
    
    file.close();
    return dims;
}