#include "pim_compiler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

// Create test files with different matrix multiplication patterns
void createTestFiles() {
    // Test file 1: Classic with #define
    std::ofstream test1("test_classic.cpp");
    test1 << "#define M 128\n";
    test1 << "#define N 64\n";
    test1 << "#define K 32\n\n";
    test1 << "void matrix_multiply(int A[][K], int B[][N], int C[][N]) {\n";
    test1 << "    for (int i = 0; i < M; i++) {\n";
    test1 << "        for (int j = 0; j < N; j++) {\n";
    test1 << "            int sum = 0;\n";
    test1 << "            for (int k = 0; k < K; k++) {\n";
    test1 << "                sum += A[i][k] * B[k][j];\n";
    test1 << "            }\n";
    test1 << "            C[i][j] = sum;\n";
    test1 << "        }\n";
    test1 << "    }\n";
    test1 << "}\n";
    test1.close();
    
    // Test file 2: Using std::vector
    std::ofstream test2("test_vector.cpp");
    test2 << "#include <vector>\n";
    test2 << "using namespace std;\n\n";
    test2 << "void matrix_multiply(vector<vector<int>>& A, vector<vector<int>>& B, vector<vector<int>>& C) {\n";
    test2 << "    int M = A.size();\n";
    test2 << "    int K = A[0].size();\n";
    test2 << "    int N = B[0].size();\n\n";
    test2 << "    for (int i = 0; i < M; i++) {\n";
    test2 << "        for (int j = 0; j < N; j++) {\n";
    test2 << "            int sum = 0;\n";
    test2 << "            for (int k = 0; k < K; k++) {\n";
    test2 << "                sum += A[i][k] * B[k][j];\n";
    test2 << "            }\n";
    test2 << "            C[i][j] = sum;\n";
    test2 << "        }\n";
    test2 << "    }\n";
    test2 << "}\n\n";
    test2 << "int main() {\n";
    test2 << "    vector<vector<int>> A(100, vector<int>(50));\n";
    test2 << "    vector<vector<int>> B(50, vector<int>(75));\n";
    test2 << "    vector<vector<int>> C(100, vector<int>(75, 0));\n";
    test2 << "    matrix_multiply(A, B, C);\n";
    test2 << "    return 0;\n";
    test2 << "}\n";
    test2.close();
    
    // Test file 3: Flattened arrays
    std::ofstream test3("test_flattened.cpp");
    test3 << "const int ROWS_A = 64;\n";
    test3 << "const int COLS_B = 64;\n";
    test3 << "const int COLS_A = 64; // Same as ROWS_B\n\n";
    test3 << "void matrix_multiply(int* A, int* B, int* C) {\n";
    test3 << "    for (int i = 0; i < ROWS_A; i++) {\n";
    test3 << "        for (int j = 0; j < COLS_B; j++) {\n";
    test3 << "            int sum = 0;\n";
    test3 << "            for (int k = 0; k < COLS_A; k++) {\n";
    test3 << "                sum += A[i * COLS_A + k] * B[k * COLS_B + j];\n";
    test3 << "            }\n";
    test3 << "            C[i * COLS_B + j] = sum;\n";
    test3 << "        }\n";
    test3 << "    }\n";
    test3 << "}\n";
    test3.close();
}

int main() {
    std::cout << "=== Testing Enhanced Matrix Multiplication Parser ===" << std::endl;
    
    // Create test files
    createTestFiles();
    
    // Test file 1: Classic with #define
    std::cout << "\nTesting classic pattern with #define..." << std::endl;
    MatrixDimensions dims1 = parseMatrixMultiplyEnhanced("test_classic.cpp");
    std::cout << "Expected: M=128, K=32, N=64" << std::endl;
    std::cout << "Got: M=" << dims1.M << ", K=" << dims1.K << ", N=" << dims1.N << std::endl;
    assert(dims1.M == 128 && dims1.K == 32 && dims1.N == 64);
    
    // Test file 2: Using std::vector
    std::cout << "\nTesting std::vector pattern..." << std::endl;
    MatrixDimensions dims2 = parseMatrixMultiplyEnhanced("test_vector.cpp");
    std::cout << "Expected: M=100, K=50, N=75" << std::endl;
    std::cout << "Got: M=" << dims2.M << ", K=" << dims2.K << ", N=" << dims2.N << std::endl;
    // Depending on your implementation, the parser might get these dimensions from the vector initialization
    
    // Test file 3: Flattened arrays
    std::cout << "\nTesting flattened array pattern..." << std::endl;
    MatrixDimensions dims3 = parseMatrixMultiplyEnhanced("test_flattened.cpp");
    std::cout << "Expected: M=64, K=64, N=64" << std::endl;
    std::cout << "Got: M=" << dims3.M << ", K=" << dims3.K << ", N=" << dims3.N << std::endl;
    assert(dims3.M == 64 && dims3.K == 64 && dims3.N == 64);
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}