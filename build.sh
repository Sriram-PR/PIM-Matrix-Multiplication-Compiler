#!/bin/bash
set -e

# Build script for Enhanced PIM Compiler

echo "=== Building Enhanced PIM Compiler ==="

# Configure and build
cd build
echo "Configuring with CMake..."
cmake ..

echo "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Compiler executable: $(pwd)/pim_compiler"
    
    # Run enhanced parser test
    if [ -f "test_enhanced_parser" ]; then
        echo ""
        echo "Running enhanced parser test..."
        ./test_enhanced_parser
    fi
else
    echo "Build failed!"
    exit 1
fi

cd ..
echo "Done."