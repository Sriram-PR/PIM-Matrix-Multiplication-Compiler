#ifndef PIM_COMPILER_H
#define PIM_COMPILER_H

// Global constants
const int MEMORY_ROW_SIZE = 512;  // Each row in memory subarray has 512 elements

#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

// Common structs used throughout the compiler

// Matrix dimensions
struct MatrixDimensions {
    int M;  // Rows in A
    int N;  // Columns in B
    int K;  // Columns in A / Rows in B
};

// Matrix information
struct MatrixInfo {
    std::string name;
    std::string type;
    bool isFlattened;
    int rows;
    int cols;
};

// Three-address code representation
struct ThreeAddressCode {
    std::vector<std::string> instructions;
};

// Work assignment for parallel processing
struct WorkAssignment {
    int coreId;
    int startRow;
    int endRow;
};

// Memory layout information
struct MemoryMap {
    int baseAddrA;
    int baseAddrB;
    int baseAddrC;
    int rowSizeA;
    int rowSizeB;
    int rowSizeC;
    // Added fields for per-row memory layout
    int rowsPerMatrixRowA;
    int rowsPerMatrixRowB;
    int rowsPerMatrixRowC;
};

// Forward declarations for main compiler components

// Parser component - extracts matrix dimensions from C++ code
// Original parser function
MatrixDimensions parseMatrixMultiply(const std::string& filename);

// Enhanced parser that can handle more matrix multiplication patterns
// IMPORTANT: This must have a different name than the original function
MatrixDimensions parseMatrixMultiplyEnhanced(const std::string& filename);

// Three-address code generator - converts matrix multiplication to 3AC
ThreeAddressCode generateThreeAddressCode(const MatrixDimensions& dims);

// Work distribution - assigns matrix portions to cores
std::vector<WorkAssignment> distributeWork(const MatrixDimensions& dims, int numCores);

// Memory layout optimizer - arranges matrices in memory
MemoryMap optimizeMemoryLayout(const MatrixDimensions& dims);

// Instruction generators for 24-bit PIM instructions following the ISA format
// Operation codes: 00=NoOp, 01=PROG, 10=EXE, 11=END at bits 18-17
std::string genNoOpInstr();
std::string genProgInstr(int coreId, bool read = true, bool write = false, int addr = 0);
std::string genExeInstr(int coreId, bool read = false, bool write = false, int addr = 0);
std::string genEndInstr(int coreId, bool read = false, bool write = false, int addr = 0);

// Core instruction sequence generator
std::vector<std::string> generateCoreInstructions(
    int coreId, int startRow, int endRow, 
    const MatrixDimensions& dims, const MemoryMap& memMap);

#endif // PIM_COMPILER_H