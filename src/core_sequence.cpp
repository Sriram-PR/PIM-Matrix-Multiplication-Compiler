#include "pim_compiler.h"
#include <iostream>

// Generate the complete instruction sequence for a single core
std::vector<std::string> generateCoreInstructions(
    int coreId, int startRow, int endRow, 
    const MatrixDimensions& dims, const MemoryMap& memMap) {
    
    std::vector<std::string> instructions;
    
    // Extract dimensions for readability
    int M = dims.M;
    int N = dims.N;
    int K = dims.K;
    
    // Add comments to show which core this is for
    instructions.push_back("# Instructions for Core " + std::to_string(coreId) + 
                          " (Rows " + std::to_string(startRow) + " to " + 
                          std::to_string(endRow) + ")");
    
    // Step 1: Program this core for matrix multiplication
    // We use a unique function ID (1 = matrix multiplication)
    instructions.push_back(genProgInstr(coreId, true, false, 1));
    
    // For each row assigned to this core
    for (int i = startRow; i <= endRow; i++) {
        // Add comment for clarity
        instructions.push_back("# Processing row " + std::to_string(i));
        
        // If a matrix row spans multiple memory rows, we need to handle it specially
        if (memMap.rowsPerMatrixRowA > 1) {
            // Base address for this matrix row in memory
            int aRowBase = memMap.baseAddrA + (i * memMap.rowsPerMatrixRowA);
            
            for (int segment = 0; segment < memMap.rowsPerMatrixRowA; segment++) {
                int aSegmentAddr = aRowBase + segment;
                
                // Calculate actual starting offset within the segment 
                // (in most cases this will be 0 except for the last segment)
                int startPos = segment * MEMORY_ROW_SIZE;
                int endPos = std::min(startPos + MEMORY_ROW_SIZE, memMap.rowSizeA);
                int elementsInSegment = endPos - startPos;
                
                // Load this memory row segment
                instructions.push_back(genExeInstr(coreId, true, false, aSegmentAddr));
                instructions.push_back(genExeInstr(coreId, false, false, 0)); // Offset is 0 for full rows
                
                // Process this segment...
                // Additional instructions for segment processing would go here
            }
        } else {
            // Simple case: one matrix row fits in one or fewer memory rows
            // Calculate memory address for row i of matrix A
            int aRowAddr = memMap.baseAddrA + (i * memMap.rowSizeA / MEMORY_ROW_SIZE);
            int aRowOffset = (i * memMap.rowSizeA) % MEMORY_ROW_SIZE;
            
            // Load row i from matrix A
            instructions.push_back(genExeInstr(coreId, true, false, aRowAddr));
            instructions.push_back(genExeInstr(coreId, false, false, aRowOffset));
        }
        
        // For each column in the output
        for (int j = 0; j < N; j++) {
            // Add comment for clarity
            instructions.push_back("# Computing element C[" + std::to_string(i) + 
                                  "][" + std::to_string(j) + "]");
            
            // Clear accumulator for this element
            instructions.push_back(genExeInstr(coreId, false, false, 0));
            
            // For each element in the dot product
            for (int k = 0; k < K; k++) {
                // Calculate address for B[k][j] using rowSizeB
                int bIndex = k * memMap.rowSizeB + j;
                int bAddr = memMap.baseAddrB + (bIndex / MEMORY_ROW_SIZE);
                int bOffset = bIndex % MEMORY_ROW_SIZE;
                
                // Load element from matrix B
                instructions.push_back(genExeInstr(coreId, true, false, bAddr));
                instructions.push_back(genExeInstr(coreId, false, false, bOffset));
                
                // Perform multiply-accumulate
                // This uses a special operation code (2 = multiply-accumulate)
                instructions.push_back(genExeInstr(coreId, false, false, 2));
            }
            
            // Calculate address for C[i][j] using rowSizeC
            int cIndex = i * memMap.rowSizeC + j;
            int cAddr = memMap.baseAddrC + (cIndex / MEMORY_ROW_SIZE);
            int cOffset = cIndex % MEMORY_ROW_SIZE;
            
            // Store result to matrix C
            instructions.push_back(genExeInstr(coreId, false, true, cAddr));
            instructions.push_back(genExeInstr(coreId, false, false, cOffset));
        }
    }
    
    // Signal completion of this core's work
    instructions.push_back(genEndInstr(coreId, false, false, 0));
    
    return instructions;
}