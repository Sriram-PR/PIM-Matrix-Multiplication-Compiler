#include "pim_compiler.h"
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <vector>

// Structure to store information about detected matrix multiplication
struct MatrixMultInfo {
    bool isMatrixMult = false;
    std::string matrixA;
    std::string matrixB;
    std::string matrixC;
    MatrixDimensions dims;
};

// Helper function to read an entire file into a string
std::string readFileContents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return "";
    }
    
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// Check for matrix dimension definitions - supports more formats
MatrixDimensions findMatrixDimensions(const std::string& code) {
    MatrixDimensions dims;
    dims.M = dims.N = dims.K = -1;  // Default to invalid dimensions
    
    // First, try looking for #define statements (original approach)
    std::regex define_pattern("#define\\s+(\\w+)\\s+(\\d+)");
    std::smatch match;
    std::string::const_iterator searchStart(code.cbegin());
    
    while (std::regex_search(searchStart, code.cend(), match, define_pattern)) {
        if (match[1] == "M" || match[1] == "ROWS_A" || match[1] == "ROWS") {
            dims.M = std::stoi(match[2]);
        } else if (match[1] == "N" || match[1] == "COLS_B" || match[1] == "COLS") {
            dims.N = std::stoi(match[2]);
        } else if (match[1] == "K" || match[1] == "COLS_A" || match[1] == "ROWS_B") {
            dims.K = std::stoi(match[2]);
        }
        searchStart = match.suffix().first;
    }
    
    // Next, look for constant declarations
    std::regex const_pattern("const\\s+\\w+\\s+(\\w+)\\s*=\\s*(\\d+)");
    searchStart = code.cbegin();
    
    while (std::regex_search(searchStart, code.cend(), match, const_pattern)) {
        if (match[1] == "M" || match[1] == "rowsA" || match[1] == "rows") {
            dims.M = std::stoi(match[2]);
        } else if (match[1] == "N" || match[1] == "colsB" || match[1] == "cols") {
            dims.N = std::stoi(match[2]);
        } else if (match[1] == "K" || match[1] == "colsA" || match[1] == "rowsB") {
            dims.K = std::stoi(match[2]);
        }
        searchStart = match.suffix().first;
    }
    
    // If still not found, look for array/vector declarations
    if (dims.M == -1 || dims.N == -1 || dims.K == -1) {
        // Look for C-style array declarations
        std::regex array_pattern("(\\w+)\\s*\\[\\s*(\\d+)\\s*\\]\\s*\\[\\s*(\\d+)\\s*\\]");
        searchStart = code.cbegin();
        
        std::vector<std::pair<std::string, std::pair<int, int>>> arrays;
        
        while (std::regex_search(searchStart, code.cend(), match, array_pattern)) {
            std::string name = match[1];
            int dim1 = std::stoi(match[2]);
            int dim2 = std::stoi(match[3]);
            arrays.push_back({name, {dim1, dim2}});
            searchStart = match.suffix().first;
        }
        
        // Look for vector declarations
        std::regex vector_pattern("vector\\s*<.*>\\s+(\\w+)\\s*\\(\\s*(\\d+)\\s*,");
        searchStart = code.cbegin();
        
        while (std::regex_search(searchStart, code.cend(), match, vector_pattern)) {
            std::string name = match[1];
            int dim = std::stoi(match[2]);
            bool found = false;
            
            // Check if this matches any array we found before
            for (auto& arr : arrays) {
                if (arr.first + "Vec" == name || arr.first + "_vec" == name) {
                    found = true;
                    // This might be a vector of vectors
                    if (dim == arr.second.first) {
                        // Assume this is matrix A
                        dims.M = dim;
                    } else if (dim == arr.second.second) {
                        // Assume this is matrix B
                        dims.N = dim;
                    }
                }
            }
            
            if (!found) {
                // This might be a new matrix
                if (name == "A" || name == "matA" || name == "a") {
                    dims.M = dim;
                } else if (name == "B" || name == "matB" || name == "b") {
                    dims.K = dim;
                } else if (name == "C" || name == "matC" || name == "c") {
                    dims.N = dim;
                }
            }
            
            searchStart = match.suffix().first;
        }
    }
    
    // If we still don't have all dimensions, try to infer from loop bounds
    if (dims.M == -1 || dims.N == -1 || dims.K == -1) {
        // Look for typical matrix multiplication loop pattern
        std::regex loop_pattern("for\\s*\\(.*?(\\w+)\\s*=\\s*0\\s*;\\s*\\1\\s*<\\s*(\\d+|\\w+)\\s*;");
        searchStart = code.cbegin();
        
        std::vector<std::pair<std::string, std::string>> loops;
        
        while (std::regex_search(searchStart, code.cend(), match, loop_pattern)) {
            std::string var = match[1];
            std::string bound = match[2];
            loops.push_back({var, bound});
            searchStart = match.suffix().first;
        }
        
        // Look for typical loop variables i, j, k
        if (loops.size() >= 3) {
            for (const auto& loop : loops) {
                // Convert bound to int if it's a number
                int boundVal = -1;
                try {
                    boundVal = std::stoi(loop.second);
                } catch (...) {
                    // It's a variable name, check if we know its value
                    if (loop.second == "M" || loop.second == "ROWS_A" || loop.second == "ROWS") {
                        boundVal = dims.M;
                    } else if (loop.second == "N" || loop.second == "COLS_B" || loop.second == "COLS") {
                        boundVal = dims.N;
                    } else if (loop.second == "K" || loop.second == "COLS_A" || loop.second == "ROWS_B") {
                        boundVal = dims.K;
                    }
                }
                
                if (boundVal != -1) {
                    // Assign to appropriate dimension based on loop variable
                    if (loop.first == "i") {
                        dims.M = boundVal;
                    } else if (loop.first == "j") {
                        dims.N = boundVal;
                    } else if (loop.first == "k") {
                        dims.K = boundVal;
                    }
                }
            }
        }
    }
    
    // Final fallback - look for specific matrix multiply function patterns
    if (dims.M == -1 || dims.N == -1 || dims.K == -1) {
        // TODO: Add more patterns as needed
    }
    
    // If we still don't have all dimensions, use default values as last resort
    if (dims.M == -1) dims.M = 64;  // Default
    if (dims.N == -1) dims.N = 64;  // Default
    if (dims.K == -1) dims.K = 64;  // Default
    
    return dims;
}

// Detect matrix multiplication patterns in code
MatrixMultInfo detectMatrixMultiplication(const std::string& code) {
    MatrixMultInfo info;
    
    // Find matrix dimensions first
    info.dims = findMatrixDimensions(code);
    
    // Look for different matrix multiplication patterns
    
    // Pattern 1: Classic triple nested loop
    std::regex pattern1(
        "for\\s*\\(.*?\\)\\s*\\{"
        "\\s*for\\s*\\(.*?\\)\\s*\\{"
        "\\s*for\\s*\\(.*?\\)\\s*\\{"
        ".*?(\\w+)\\s*\\[.*?\\]\\s*\\[.*?\\]\\s*[\\+]*=\\s*(\\w+)\\s*\\[.*?\\]\\s*\\[.*?\\]\\s*\\*\\s*(\\w+)\\s*\\[.*?\\]\\s*\\[.*?\\]"
    );
    
    std::smatch match;
    if (std::regex_search(code, match, pattern1)) {
        info.isMatrixMult = true;
        info.matrixC = match[1];
        info.matrixA = match[2];
        info.matrixB = match[3];
        return info;
    }
    
    // Pattern 2: Flattened arrays with explicit indexing
    std::regex pattern2(
        "for\\s*\\(.*?\\)\\s*\\{"
        "\\s*for\\s*\\(.*?\\)\\s*\\{"
        "\\s*for\\s*\\(.*?\\)\\s*\\{"
        ".*?(\\w+)\\s*\\[\\s*\\w+\\s*\\*\\s*\\w+\\s*\\+\\s*\\w+\\s*\\]\\s*[\\+]*=\\s*"
        "(\\w+)\\s*\\[.*?\\]\\s*\\*\\s*(\\w+)\\s*\\[.*?\\]"
    );
    
    if (std::regex_search(code, match, pattern2)) {
        info.isMatrixMult = true;
        info.matrixC = match[1];
        info.matrixA = match[2];
        info.matrixB = match[3];
        return info;
    }
    
    // Pattern 3: Vector-based matrices
    std::regex pattern3(
        "for\\s*\\(.*?\\)\\s*\\{"
        "\\s*for\\s*\\(.*?\\)\\s*\\{"
        ".*?int\\s+sum\\s*=\\s*0;"
        "\\s*for\\s*\\(.*?\\)\\s*\\{"
        "\\s*sum\\s*\\+=\\s*(\\w+)\\s*\\[.*?\\]\\s*\\*\\s*(\\w+)\\s*\\[.*?\\]"
    );
    
    if (std::regex_search(code, match, pattern3)) {
        info.isMatrixMult = true;
        info.matrixA = match[1];
        info.matrixB = match[2];
        
        // Try to find where sum is assigned
        std::regex resultPattern("sum\\s*;?\\s*\\}\\s*\\}?\\s*(\\w+)\\s*\\[.*?\\]\\s*=\\s*sum");
        std::smatch resultMatch;
        if (std::regex_search(code, resultMatch, resultPattern)) {
            info.matrixC = resultMatch[1];
        } else {
            info.matrixC = "C"; // Default name if not found
        }
        
        return info;
    }
    
    // If no pattern matched, this might not be matrix multiplication
    return info;
}

// Main entry point that combines all detection logic
MatrixDimensions parseMatrixMultiplyEnhanced(const std::string& filename) {
    std::string code = readFileContents(filename);
    if (code.empty()) {
        // If file couldn't be read, return default dimensions
        MatrixDimensions dims;
        dims.M = dims.N = dims.K = 64;
        return dims;
    }
    
    // Detect matrix multiplication
    MatrixMultInfo info = detectMatrixMultiplication(code);
    
    // Report findings
    if (info.isMatrixMult) {
        std::cout << "Detected matrix multiplication:" << std::endl;
        std::cout << "  Matrix A: " << info.matrixA << std::endl;
        std::cout << "  Matrix B: " << info.matrixB << std::endl;
        std::cout << "  Result C: " << info.matrixC << std::endl;
    } else {
        std::cout << "Warning: Could not definitively identify matrix multiplication pattern." << std::endl;
        std::cout << "Using detected or default dimensions." << std::endl;
    }
    
    std::cout << "Matrix dimensions: " << info.dims.M << "x" << info.dims.K << " * " 
              << info.dims.K << "x" << info.dims.N << std::endl;
    
    return info.dims;
}