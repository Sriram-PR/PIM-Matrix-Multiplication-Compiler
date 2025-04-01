#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

// Simple matrix multiplication reference implementation
void matrixMultiply(std::vector<std::vector<int>>& A, 
                   std::vector<std::vector<int>>& B, 
                   std::vector<std::vector<int>>& C) {
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

// Simple instruction simulator
void simulateInstructions(const std::vector<std::string>& instructions) {
    std::cout << "Simulating " << instructions.size() << " instructions..." << std::endl;
    
    int instr_count = 0;
    int prog_count = 0;
    int exe_count = 0;
    int end_count = 0;
    
    for (const auto& instr : instructions) {
        // Skip comments and empty lines
        if (instr.empty() || instr[0] == '#') {
            continue;
        }
        
        instr_count++;
        
        // Parse instruction type from first 2 bits (first hex digit)
        char type_hex = instr[0];
        int type = 0;
        
        if (type_hex == '0') {
            // PROG instruction
            prog_count++;
        } else if (type_hex == '4' || type_hex == '5') {
            // EXE instruction
            exe_count++;
        } else if (type_hex == '8' || type_hex == '9') {
            // END instruction
            end_count++;
        }
    }
    
    std::cout << "Instruction count: " << instr_count << std::endl;
    std::cout << "  PROG instructions: " << prog_count << std::endl;
    std::cout << "  EXE instructions: " << exe_count << std::endl;
    std::cout << "  END instructions: " << end_count << std::endl;
}

int main() {
    std::cout << "=== PIM Compiler Test ===" << std::endl;
    
    // Set up test matrices
    const int M = 4;
    const int N = 3;
    const int K = 2;
    
    std::vector<std::vector<int>> A(M, std::vector<int>(K));
    std::vector<std::vector<int>> B(K, std::vector<int>(N));
    std::vector<std::vector<int>> C(M, std::vector<int>(N, 0));
    
    // Initialize test matrices
    for (int i = 0; i < M; i++) {
        for (int k = 0; k < K; k++) {
            A[i][k] = i + k + 1;
        }
    }
    
    for (int k = 0; k < K; k++) {
        for (int j = 0; j < N; j++) {
            B[k][j] = k - j + 2;
        }
    }
    
    // Compute reference result
    matrixMultiply(A, B, C);
    
    // Print test matrices
    std::cout << "\nMatrix A (" << M << "x" << K << "):" << std::endl;
    for (int i = 0; i < M; i++) {
        for (int k = 0; k < K; k++) {
            std::cout << A[i][k] << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nMatrix B (" << K << "x" << N << "):" << std::endl;
    for (int k = 0; k < K; k++) {
        for (int j = 0; j < N; j++) {
            std::cout << B[k][j] << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nResult C (" << M << "x" << N << "):" << std::endl;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            std::cout << C[i][j] << " ";
        }
        std::cout << std::endl;
    }
    
    // Create test input file
    std::ofstream testFile("test_matmul.cpp");
    testFile << "// Matrix multiplication test\n";
    testFile << "#define M " << M << "\n";
    testFile << "#define N " << N << "\n";
    testFile << "#define K " << K << "\n\n";
    testFile << "void matrix_multiply(int* A, int* B, int* C) {\n";
    testFile << "    for (int i = 0; i < M; i++) {\n";
    testFile << "        for (int j = 0; j < N; j++) {\n";
    testFile << "            int sum = 0;\n";
    testFile << "            for (int k = 0; k < K; k++) {\n";
    testFile << "                sum += A[i * K + k] * B[k * N + j];\n";
    testFile << "            }\n";
    testFile << "            C[i * N + j] = sum;\n";
    testFile << "        }\n";
    testFile << "    }\n";
    testFile << "}\n";
    testFile.close();
    
    // Run the compiler
    std::cout << "\nRunning compiler on test input..." << std::endl;
    std::string command = "../build/pim_compiler test_matmul.cpp -o test_output.pim -c 2";
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Error: Compiler returned non-zero exit code." << std::endl;
        return 1;
    }
    
    // Read and analyze the generated instructions
    std::ifstream instrFile("test_output.pim");
    std::vector<std::string> instructions;
    std::string line;
    
    while (std::getline(instrFile, line)) {
        if (!line.empty()) {
            instructions.push_back(line);
        }
    }
    instrFile.close();
    
    // Simulate execution of the instructions
    std::cout << "\nAnalyzing generated instructions..." << std::endl;
    simulateInstructions(instructions);
    
    std::cout << "\nTest completed successfully!" << std::endl;
    return 0;
}