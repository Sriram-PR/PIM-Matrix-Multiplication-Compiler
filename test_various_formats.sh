#!/bin/bash
set -e

# Testing script for the enhanced PIM compiler
# Tests various formats of matrix multiplication

echo "=== Testing Enhanced PIM Compiler with Various Matrix Multiplication Formats ==="

# Ensure the compiler is built
if [ ! -f "build/pim_compiler" ]; then
    echo "Compiler not found. Building first..."
    ./build_enhanced.sh
fi

# Create a directory for test outputs
mkdir -p test_outputs

# Test 1: Standard matrix multiplication (from examples)
echo ""
echo "Test 1: Standard matrix multiplication"
echo "--------------------------------------"
build/pim_compiler examples/matrix_multiply.cpp -o test_outputs/output_standard.pim -c 2
echo ""

# Test 2: Custom matrix multiplication
echo "Test 2: Custom matrix multiplication pattern"
echo "-------------------------------------------"
build/pim_compiler custom_matmul.cpp -o test_outputs/output_custom.pim -c 2
echo ""

# Test 3: Test classic pattern
echo "Test 3: Classic pattern with #define"
echo "-----------------------------------"
build/pim_compiler test_classic.cpp -o test_outputs/output_classic.pim -c 2
echo ""

# Test 4: Vector pattern
echo "Test 4: Vector-based matrix multiplication"
echo "----------------------------------------"
build/pim_compiler test_vector.cpp -o test_outputs/output_vector.pim -c 2
echo ""

# Test 5: Flattened array pattern
echo "Test 5: Flattened array pattern"
echo "-----------------------------"
build/pim_compiler test_flattened.cpp -o test_outputs/output_flattened.pim -c 2
echo ""

echo "All tests completed! Check the test_outputs directory for results."