#!/bin/bash
set -e

# ==========================================================================
# Consolidated Test Script for PIM Matrix Multiplication Compiler
# Combines functionality from:
#   - run_tests.sh
#   - run_example.sh
#   - test_various_formats.sh
# ==========================================================================

# Color definitions for better readability
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Create directory for test outputs
mkdir -p test_outputs

# Parse command line arguments
TEST_PARSER=true
TEST_SIZES=true
TEST_FORMATS=true
TEST_ALL=true

# Check if specific tests are requested
if [ $# -gt 0 ]; then
    TEST_ALL=false
    for arg in "$@"; do
        case $arg in
            --parser) TEST_PARSER=true ;;
            --sizes) TEST_SIZES=true ;;
            --formats) TEST_FORMATS=true ;;
            --all) TEST_ALL=true ;;
            --help)
                echo "Usage: $0 [OPTIONS]"
                echo "Options:"
                echo "  --parser   Test the parser components"
                echo "  --sizes    Test different matrix sizes"
                echo "  --formats  Test different matrix multiplication formats"
                echo "  --all      Run all tests (default if no options provided)"
                echo "  --help     Show this help message"
                exit 0
                ;;
            *)
                echo "Unknown option: $arg"
                echo "Use --help for usage information"
                exit 1
                ;;
        esac
    done
fi

# =========================================================================
# Function to build the project
# =========================================================================
build_project() {
    printf "${BLUE}=== Building PIM Compiler ===${NC}\n"
    
    # Create build directory if it doesn't exist
    mkdir -p build
    cd build
    
    # Configure and build
    echo "Configuring with CMake..."
    cmake ..
    
    echo "Building..."
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo "Build successful!"
        echo "Compiler executable: $(pwd)/pim_compiler"
    else
        echo "Build failed!"
        exit 1
    fi
    
    cd ..
}

# =========================================================================
# Function to create test files with different matrix multiplication patterns
# =========================================================================
create_test_files() {
    printf "${BLUE}=== Creating Test Files ===${NC}\n"
    
    # Create test_files directory if it doesn't exist
    mkdir -p test_files
    
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
    echo "Created test_files/test_classic.cpp"
    
    # Test file 2: Using std::vector
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
    echo "Created test_files/test_vector.cpp"
    
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
    echo "Created test_files/test_flattened.cpp"
    
    # Test file 4: Custom loop ordering (kij)
    cat > test_files/test_kij_ordering.cpp << EOF
const int M = 64;
const int K = 64;
const int N = 64;

void matrix_multiply(double* A, double* B, double* C) {
    // Zero initialize the result
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0.0;
        }
    }
    
    // kij loop ordering
    for (int k = 0; k < K; k++) {
        for (int i = 0; i < M; i++) {
            double r = A[i * K + k];
            for (int j = 0; j < N; j++) {
                C[i * N + j] += r * B[k * N + j];
            }
        }
    }
}
EOF
    echo "Created test_files/test_kij_ordering.cpp"
}

# =========================================================================
# Function to test parser components
# =========================================================================
test_parser() {
    printf "${BLUE}=== Testing Parser Components ===${NC}\n"
    
    # Ensure compiler and test executable exist
    if [ ! -f "build/pim_compiler" ] || [ ! -f "build/test_enhanced_parser" ]; then
        build_project
    fi
    
    # Run the enhanced parser test
    printf "${YELLOW}Running enhanced parser test...${NC}\n"
    cd build
    ./test_enhanced_parser
    cd ..
    
    # Create test files if they don't exist
    if [ ! -d "test_files" ]; then
        create_test_files
    fi
    
    # Test original parser vs enhanced parser
    printf "${YELLOW}Testing original parser vs enhanced parser...${NC}\n"
    
    # Basic parser with classic pattern
    printf "${GREEN}Testing original parser with classic pattern...${NC}\n"
    build/pim_compiler test_files/test_classic.cpp -o test_outputs/output_original.pim -p 0
    
    # Enhanced parser with various patterns
    printf "${GREEN}Testing enhanced parser with classic pattern...${NC}\n"
    build/pim_compiler test_files/test_classic.cpp -o test_outputs/output_enhanced_classic.pim -p 1
    
    printf "${GREEN}Testing enhanced parser with vector pattern...${NC}\n"
    build/pim_compiler test_files/test_vector.cpp -o test_outputs/output_enhanced_vector.pim -p 1
    
    printf "${GREEN}Testing enhanced parser with flattened pattern...${NC}\n"
    build/pim_compiler test_files/test_flattened.cpp -o test_outputs/output_enhanced_flattened.pim -p 1
    
    printf "${GREEN}Testing enhanced parser with kij loop ordering...${NC}\n"
    build/pim_compiler test_files/test_kij_ordering.cpp -o test_outputs/output_enhanced_kij.pim -p 1
    
    echo "Parser tests completed. Results saved to test_outputs directory."
}

# =========================================================================
# Function to test different matrix sizes
# =========================================================================
test_matrix_sizes() {
    printf "${BLUE}=== Testing Different Matrix Sizes ===${NC}\n"
    
    # Ensure compiler exists
    if [ ! -f "build/pim_compiler" ]; then
        build_project
    fi
    
    # Matrices to test
    SIZES=(
        "4 4 4"     # Small (4x4 * 4x4)
        "16 16 16"  # Medium (16x16 * 16x16)
        "64 64 64"  # Large (64x64 * 64x64)
        "256 128 64" # Rectangular (256x128 * 128x64)
    )
    
    for size in "${SIZES[@]}"; do
        read -r M K N <<< "$size"
        
        printf "${GREEN}Testing matrix size: $M x $K * $K x $N${NC}\n"
        
        # Run the compiler with specific dimensions
        OUTPUT_FILE="test_outputs/size_${M}_${K}_${N}.pim"
        
        build/pim_compiler examples/matrix_multiply.cpp -o "$OUTPUT_FILE" -M "$M" -K "$K" -N "$N" -c 4
        
        # Count actual instructions
        INSTR_COUNT=$(grep -v "^#" "$OUTPUT_FILE" | grep -v "^$" | wc -l)
        
        echo "Generated $INSTR_COUNT instructions in $OUTPUT_FILE"
        echo "Sample instructions:"
        head -n 5 "$OUTPUT_FILE"
        echo "..."
    done
    
    echo "Matrix size tests completed. Results saved to test_outputs directory."
}

# =========================================================================
# Function to test different matrix multiplication formats
# =========================================================================
test_matrix_formats() {
    printf "${BLUE}=== Testing Different Matrix Multiplication Formats ===${NC}\n"
    
    # Ensure compiler exists
    if [ ! -f "build/pim_compiler" ]; then
        build_project
    fi
    
    # Create test files if they don't exist
    if [ ! -d "test_files" ]; then
        create_test_files
    fi
    
    # Test files to process
    FILES=(
        "examples/matrix_multiply.cpp:standard:Standard matrix multiplication"
        "test_files/test_classic.cpp:classic:Classic pattern with #define"
        "test_files/test_vector.cpp:vector:Vector-based matrix multiplication"
        "test_files/test_flattened.cpp:flattened:Flattened array pattern"
        "test_files/test_kij_ordering.cpp:kij:KIJ loop ordering"
    )
    
    # Process each file
    for file_info in "${FILES[@]}"; do
        IFS=':' read -r file_path output_name description <<< "$file_info"
        
        printf "${GREEN}Testing $description${NC}\n"
        
        # Run the compiler
        OUTPUT_FILE="test_outputs/format_${output_name}.pim"
        build/pim_compiler "$file_path" -o "$OUTPUT_FILE" -c 2
        
        # Count actual instructions
        INSTR_COUNT=$(grep -v "^#" "$OUTPUT_FILE" | grep -v "^$" | wc -l)
        
        echo "Generated $INSTR_COUNT instructions for $description"
        echo "First few instructions:"
        head -n 5 "$OUTPUT_FILE"
        echo "..."
    done
    
    echo "Matrix format tests completed. Results saved to test_outputs directory."
}

# =========================================================================
# Function to run the simulator on a generated instruction file
# =========================================================================
run_simulator() {
    local instruction_file=$1
    local matrix_size=$2
    
    printf "${BLUE}=== Running Simulator for $instruction_file (Size: $matrix_size) ===${NC}\n"
    
    if [ -f "$instruction_file" ]; then
        python3 pim_simulator.py "$instruction_file" --deterministic --no-validate
        
        # Ask if user wants to see full validation
        read -p "Run full validation? (y/n): " validate
        if [[ $validate == "y" || $validate == "Y" ]]; then
            python3 pim_simulator.py "$instruction_file" --deterministic
        fi
    else
        echo "Error: Instruction file $instruction_file not found!"
    fi
}

# =========================================================================
# Main test flow
# =========================================================================

# Ensure the compiler is built
if [ ! -f "build/pim_compiler" ]; then
    build_project
fi

# Run tests based on command line arguments
if [ "$TEST_ALL" = true ] || [ "$TEST_PARSER" = true ]; then
    test_parser
fi

if [ "$TEST_ALL" = true ] || [ "$TEST_SIZES" = true ]; then
    test_matrix_sizes
fi

if [ "$TEST_ALL" = true ] || [ "$TEST_FORMATS" = true ]; then
    test_matrix_formats
fi

# Summary of results
printf "${BLUE}=== Test Summary ===${NC}\n"
echo "All requested tests completed successfully!"
echo "Test outputs are available in the test_outputs directory:"
ls -la test_outputs/

# Ask if user wants to run the simulator on any of the generated files
echo ""
if [ -f "pim_simulator.py" ]; then
    read -p "Would you like to run the simulator on one of the output files? (y/n): " run_sim
    if [[ $run_sim == "y" || $run_sim == "Y" ]]; then
        echo "Available output files:"
        ls test_outputs/*.pim | cat -n
        read -p "Enter the number of the file to simulate: " file_num
        
        selected_file=$(ls test_outputs/*.pim | sed -n "${file_num}p")
        if [ -n "$selected_file" ]; then
            read -p "Enter matrix size (e.g., '4 4 4' for 4x4*4x4): " matrix_size
            run_simulator "$selected_file" "$matrix_size"
        else
            echo "Invalid file number."
        fi
    fi
fi

echo "Done."