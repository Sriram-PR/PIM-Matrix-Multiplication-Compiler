#include "pim_compiler.h"
#include <iostream>

MemoryMap optimizeMemoryLayout(const MatrixDimensions& dims) {
    MemoryMap map;
    // MEMORY_ROW_SIZE is now defined in the header
    
    // Calculate total elements in each matrix
    int sizeA = dims.M * dims.K;
    int sizeB = dims.K * dims.N;
    int sizeC = dims.M * dims.N;
    
    // Calculate how many memory rows each matrix requires
    int rowsA = (sizeA + MEMORY_ROW_SIZE - 1) / MEMORY_ROW_SIZE;
    int rowsB = (sizeB + MEMORY_ROW_SIZE - 1) / MEMORY_ROW_SIZE;
    int rowsC = (sizeC + MEMORY_ROW_SIZE - 1) / MEMORY_ROW_SIZE;
    
    // Calculate how many memory rows each matrix row requires
    map.rowsPerMatrixRowA = (dims.K + MEMORY_ROW_SIZE - 1) / MEMORY_ROW_SIZE;
    map.rowsPerMatrixRowB = (dims.N + MEMORY_ROW_SIZE - 1) / MEMORY_ROW_SIZE;
    map.rowsPerMatrixRowC = (dims.N + MEMORY_ROW_SIZE - 1) / MEMORY_ROW_SIZE;
    
    // Assign base addresses (row numbers)
    map.baseAddrA = 0;
    map.baseAddrB = rowsA;
    map.baseAddrC = rowsA + rowsB;
    
    // Store individual row sizes
    map.rowSizeA = dims.K;  // Each row of A has K elements
    map.rowSizeB = dims.N;  // Each row of B has N elements
    map.rowSizeC = dims.N;  // Each row of C has N elements
    
    std::cout << "Memory layout:" << std::endl;
    std::cout << "  Matrix A: Base address = " << map.baseAddrA << ", size = " 
              << sizeA << " elements (" << rowsA << " rows)" << std::endl;
    std::cout << "  Matrix B: Base address = " << map.baseAddrB << ", size = " 
              << sizeB << " elements (" << rowsB << " rows)" << std::endl;
    std::cout << "  Matrix C: Base address = " << map.baseAddrC << ", size = " 
              << sizeC << " elements (" << rowsC << " rows)" << std::endl;
    std::cout << "  Matrix row layout: A=" << map.rowsPerMatrixRowA 
              << ", B=" << map.rowsPerMatrixRowB 
              << ", C=" << map.rowsPerMatrixRowC 
              << " memory rows per matrix row" << std::endl;
    
    return map;
}