#include "pim_compiler.h"
#include <sstream>

ThreeAddressCode generateThreeAddressCode(const MatrixDimensions& dims) {
    ThreeAddressCode code;
    std::vector<std::string>& instructions = code.instructions;
    
    // Convenience variables for readability
    int M = dims.M;
    int N = dims.N;
    int K = dims.K;
    
    // Initialize loop counter i
    instructions.push_back("i = 0");
    instructions.push_back("L1: if i >= " + std::to_string(M) + " goto END_L1");
    
    // Begin loop for each row in matrix A
    instructions.push_back("    j = 0");
    instructions.push_back("    L2: if j >= " + std::to_string(N) + " goto END_L2");
    
    // Begin loop for each column in matrix B
    instructions.push_back("        sum = 0");
    instructions.push_back("        k = 0");
    instructions.push_back("        L3: if k >= " + std::to_string(K) + " goto END_L3");
    
    // Inner loop calculations
    instructions.push_back("            t1 = i * " + std::to_string(K));
    instructions.push_back("            t2 = t1 + k");
    instructions.push_back("            t3 = k * " + std::to_string(N));
    instructions.push_back("            t4 = t3 + j");
    
    // Load values from matrices
    instructions.push_back("            t5 = A[t2]");
    instructions.push_back("            t6 = B[t4]");
    
    // Multiply and accumulate
    instructions.push_back("            t7 = t5 * t6");
    instructions.push_back("            sum = sum + t7");
    
    // Increment inner loop
    instructions.push_back("            k = k + 1");
    instructions.push_back("            goto L3");
    instructions.push_back("        END_L3:");
    
    // Store result to matrix C
    instructions.push_back("        t8 = i * " + std::to_string(N));
    instructions.push_back("        t9 = t8 + j");
    instructions.push_back("        C[t9] = sum");
    
    // Increment middle loop
    instructions.push_back("        j = j + 1");
    instructions.push_back("        goto L2");
    instructions.push_back("    END_L2:");
    
    // Increment outer loop
    instructions.push_back("    i = i + 1");
    instructions.push_back("    goto L1");
    instructions.push_back("END_L1:");
    
    return code;
}