#!/bin/bash
set -e

echo "=== Interactive PIM Matrix Compilation ==="

if [ ! -f "build/pim_compiler" ]; then
    echo "Compiler not found. Building first..."
    ./build.sh
fi

echo ""
read -p "Enter number of rows in matrix A (M): " M
read -p "Enter number of columns in matrix A / rows in matrix B (K): " K
read -p "Enter number of columns in matrix B (N): " N
read -p "Enter number of cores to use: " CORES
read -p "Enter output filename [output.pim]: " OUTPUT_FILE

if [ -z "$OUTPUT_FILE" ]; then
    OUTPUT_FILE="output.pim"
fi

echo ""
echo "Running compiler with M=$M, K=$K, N=$N, cores=$CORES..."
build/pim_compiler examples/matrix_multiply.cpp -o "$OUTPUT_FILE" -M "$M" -N "$N" -K "$K" -c "$CORES"

if [ $? -eq 0 ]; then
    echo ""
    echo "PIM instructions have been generated successfully!"
    echo "Output file: $OUTPUT_FILE"
    
    echo ""
    echo "Sample of generated instructions:"
    echo "--------------------------------"
    head -n 20 "$OUTPUT_FILE"
    echo "..."
    
    echo ""
    read -p "Run simulator with these instructions? (y/n): " run_sim
    if [[ $run_sim == "y" || $run_sim == "Y" ]]; then
        echo "Running PIM simulator..."
        python3 pim_simulator.py $OUTPUT_FILE
    fi
else
    echo "Compilation failed!"
    exit 1
fi

echo ""
echo "Interactive compilation completed."