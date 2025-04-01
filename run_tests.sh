#!/bin/bash
set -e

# Comprehensive test script for PIM Compiler

echo "=== PIM Compiler Test Suite ==="


# Step 3: Create test files
echo "Creating test files..."

# Test file 1: Classic with #define
cat > test_files/test_classic.cpp << EOF
#define M 128
#define N 64
#define K 32

void matrix_multiply(int A[][K], int B[][N], int C[][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < K; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}
EOF

# Test file 2: Vector-based
cat > test_files/test_vector.cpp << EOF
#include <vector>
using namespace std;

void matrix_multiply(vector<vector<int>>& A, vector<vector<int>>& B, vector<vector<int>>& C) {
    int M = A.size();
    int K = A[0].size();
    int N = B[0].size();

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < K; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    vector<vector<int>> A(100, vector<int>(50));
    vector<vector<int>> B(50, vector<int>(75));
    vector<vector<int>> C(100, vector<int>(75, 0));
    matrix_multiply(A, B, C);
    return 0;
}
EOF

# Test file 3: Flattened arrays
cat > test_files/test_flattened.cpp << EOF
const int ROWS_A = 64;
const int COLS_B = 64;
const int COLS_A = 64; // Same as ROWS_B

void matrix_multiply(int* A, int* B, int* C) {
    for (int i = 0; i < ROWS_A; i++) {
        for (int j = 0; j < COLS_B; j++) {
            int sum = 0;
            for (int k = 0; k < COLS_A; k++) {
                sum += A[i * COLS_A + k] * B[k * COLS_B + j];
            }
            C[i * COLS_B + j] = sum;
        }
    }
}
EOF

# Step 4: Build the project
echo "Building the project..."
cd build
cmake ..
make -j$(nproc)

# Step 5: Run the tests
if [ -f "test_enhanced_parser" ]; then
    echo ""
    echo "Running enhanced parser test..."
    ./test_enhanced_parser
fi

# Step 6: Run the compiler on different inputs
echo ""
echo "Testing compiler with different matrix multiplication inputs..."

mkdir -p ../test_outputs

# Test with original parser
if [ -f "pim_compiler" ]; then
    echo ""
    echo "Testing original parser with classic pattern..."
    ./pim_compiler ../test_files/test_classic.cpp -o ../test_outputs/output_original.pim -p 0
    
    echo ""
    echo "Testing enhanced parser with classic pattern..."
    ./pim_compiler ../test_files/test_classic.cpp -o ../test_outputs/output_enhanced_classic.pim -p 1
    
    echo ""
    echo "Testing enhanced parser with vector pattern..."
    ./pim_compiler ../test_files/test_vector.cpp -o ../test_outputs/output_enhanced_vector.pim -p 1
    
    echo ""
    echo "Testing enhanced parser with flattened pattern..."
    ./pim_compiler ../test_files/test_flattened.cpp -o ../test_outputs/output_enhanced_flattened.pim -p 1
fi

echo ""
echo "All tests completed!"
echo "Check test_outputs directory for compiler results."

cd ..