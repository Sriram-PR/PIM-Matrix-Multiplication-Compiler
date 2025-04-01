#!/bin/bash
set -e

# Run script for PIM Compiler

echo "=== Running PIM Compiler Example ==="

# Ensure build exists
if [ ! -f "build/pim_compiler" ]; then
    echo "Compiler not found. Building first..."
    ./build.sh
fi

# Matrices to test
SIZES=(
    "4 4 4"    # Small
    "16 16 16" # Medium
    "64 64 64" # Large
)

for size in "${SIZES[@]}"; do
    read -r M N K <<< "$size"
    
    echo ""
    echo "Compiling matrix multiplication for size $M x $K * $K x $N..."
    
    # Run the compiler
    OUTPUT_FILE="output_${M}_${N}_${K}.pim"
    
    build/pim_compiler examples/matrix_multiply.cpp -o "$OUTPUT_FILE" -M "$M" -N "$N" -K "$K" -c 4
    
    # Count actual instructions
    INSTR_COUNT=$(grep -v "^#" "$OUTPUT_FILE" | grep -v "^$" | wc -l)
    
    echo "Generated $INSTR_COUNT instructions in $OUTPUT_FILE"
    echo "Sample instructions:"
    head -n 20 "$OUTPUT_FILE"
    echo "..."
done

echo ""
echo "All examples completed successfully."