#include "pim_compiler.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <bitset>

// Convert hex string to binary string for verification
std::string hexToBinary(const std::string& hex) {
    std::string binary;
    for (char c : hex) {
        // Convert each hex character to 4 binary digits
        switch(toupper(c)) {
            case '0': binary += "0000"; break;
            case '1': binary += "0001"; break;
            case '2': binary += "0010"; break;
            case '3': binary += "0011"; break;
            case '4': binary += "0100"; break;
            case '5': binary += "0101"; break;
            case '6': binary += "0110"; break;
            case '7': binary += "0111"; break;
            case '8': binary += "1000"; break;
            case '9': binary += "1001"; break;
            case 'A': binary += "1010"; break;
            case 'B': binary += "1011"; break;
            case 'C': binary += "1100"; break;
            case 'D': binary += "1101"; break;
            case 'E': binary += "1110"; break;
            case 'F': binary += "1111"; break;
            default: binary += "????"; // For any invalid characters
        }
    }
    return binary;
}

// Write three-address code to a separate file
void writeThreeAddressCodeToFile(const ThreeAddressCode& tac, const std::string& filename) {
    std::ofstream tacFile(filename);
    if (!tacFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing three-address code." << std::endl;
        return;
    }
    
    tacFile << "# Three-Address Code for Matrix Multiplication" << std::endl;
    tacFile << "# =====================================" << std::endl << std::endl;
    
    for (const auto& instr : tac.instructions) {
        tacFile << instr << std::endl;
    }
    
    tacFile.close();
    std::cout << "Three-address code written to " << filename << std::endl;
}

void printHelp(const char* programName) {
    std::cout << "PIM Matrix Multiplication Compiler" << std::endl;
    std::cout << "Usage: " << programName << " <input_file> [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -o <file>       Output file (default: output.pim)" << std::endl;
    std::cout << "  -M <value>      Rows in matrix A (overrides value in input file)" << std::endl;
    std::cout << "  -N <value>      Columns in matrix B (overrides value in input file)" << std::endl;
    std::cout << "  -K <value>      Columns in matrix A / Rows in matrix B (overrides value in input file)" << std::endl;
    std::cout << "  -c <value>      Number of cores to use (default: 4)" << std::endl;
    std::cout << "  -p <value>      Parser to use (0=basic, 1=enhanced [default])" << std::endl;
    std::cout << "  -h, --help      Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    // Default values
    std::string inputFile = "";
    std::string outputFile = "output.pim";
    int numCores = 4;
    int overrideM = -1;  // -1 means use value from file
    int overrideN = -1;
    int overrideK = -1;
    int parserType = 1;  // Default to enhanced parser
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "-M" && i + 1 < argc) {
            overrideM = std::stoi(argv[++i]);
        } else if (arg == "-N" && i + 1 < argc) {
            overrideN = std::stoi(argv[++i]);
        } else if (arg == "-K" && i + 1 < argc) {
            overrideK = std::stoi(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) {
            numCores = std::stoi(argv[++i]);
        } else if (arg == "-p" && i + 1 < argc) {
            parserType = std::stoi(argv[++i]);
        } else if (inputFile.empty()) {
            inputFile = arg;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printHelp(argv[0]);
            return 1;
        }
    }
    
    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        printHelp(argv[0]);
        return 1;
    }
    
    // Start timing
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "=== PIM Matrix Multiplication Compiler ===" << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    std::cout << "Number of cores: " << numCores << std::endl;
    std::cout << "Parser type: " << (parserType == 0 ? "Basic" : "Enhanced") << std::endl;
    
    // Step 1: Parse the input file to get matrix dimensions
    MatrixDimensions dims;
    
    // Use selected parser type
    if (parserType == 0) {
        dims = parseMatrixMultiply(inputFile);  // Original parser
    } else {
        dims = parseMatrixMultiplyEnhanced(inputFile);  // Enhanced parser with different function name
    }

    if (dims.M <= 0 || dims.N <= 0 || dims.K <= 0) {
        std::cerr << "Error: Invalid matrix dimensions: " << dims.M << "x" << dims.K 
                  << " * " << dims.K << "x" << dims.N << std::endl;
        return 1;
    }    
    
    // Override dimensions if specified on command line
    if (overrideM > 0) dims.M = overrideM;
    if (overrideN > 0) dims.N = overrideN;
    if (overrideK > 0) dims.K = overrideK;
    
    // Step 2: Generate three-address code
    std::cout << "\nGenerating three-address code..." << std::endl;
    ThreeAddressCode threeAddressCode = generateThreeAddressCode(dims);
    
    // Write three-address code to a separate file
    std::string tacFilename = outputFile + ".tac";
    writeThreeAddressCodeToFile(threeAddressCode, tacFilename);
    
    // Step 3: Distribute work among cores
    std::cout << "\nDistributing work among cores..." << std::endl;
    std::vector<WorkAssignment> workAssignments = distributeWork(dims, numCores);
    
    // Step 4: Optimize memory layout
    std::cout << "\nOptimizing memory layout..." << std::endl;
    MemoryMap memoryMap = optimizeMemoryLayout(dims);
    
    // Step 5: Generate PIM instructions for each core
    std::cout << "\nGenerating PIM instructions..." << std::endl;
    std::vector<std::string> allInstructions;
    
    // Add header comment
    allInstructions.push_back("# PIM Instructions for Matrix Multiplication");
    allInstructions.push_back("# Matrix dimensions: " + std::to_string(dims.M) + "x" + 
                             std::to_string(dims.K) + " * " + std::to_string(dims.K) + 
                             "x" + std::to_string(dims.N));
    allInstructions.push_back("# Using " + std::to_string(workAssignments.size()) + " cores");
    allInstructions.push_back("");
    
    // Generate instructions for each core
    for (const auto& work : workAssignments) {
        std::vector<std::string> coreInstructions = generateCoreInstructions(
            work.coreId, work.startRow, work.endRow, dims, memoryMap);
        
        // Add a blank line between cores for readability
        if (!allInstructions.empty() && !allInstructions.back().empty()) {
            allInstructions.push_back("");
        }
        
        // Add this core's instructions to the master list
        allInstructions.insert(allInstructions.end(), 
                              coreInstructions.begin(), 
                              coreInstructions.end());
    }
    
    // Step 6: Write instructions to output file
    std::cout << "\nWriting " << allInstructions.size() << " instructions to " 
              << outputFile << "..." << std::endl;
    
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFile << std::endl;
        return 1;
    }
    
    for (const auto& instr : allInstructions) {
        if (!instr.empty() && instr[0] != '#') {
            // This is an actual instruction, not a comment
            outFile << instr << " # Binary: " << hexToBinary(instr) << std::endl;
        } else {
            // This is a comment or empty line
            outFile << instr << std::endl;
        }
    }
    outFile.close();
    
    // Calculate and display statistics
    int dataInstructions = 0;
    for (const auto& instr : allInstructions) {
        if (!instr.empty() && instr[0] != '#') {
            dataInstructions++;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\nCompilation complete!" << std::endl;
    std::cout << "Total instructions: " << allInstructions.size() << " (including " 
              << (allInstructions.size() - dataInstructions) << " comments)" << std::endl;
    std::cout << "Actual instructions: " << dataInstructions << std::endl;
    std::cout << "Three-address code available in: " << tacFilename << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    
    return 0;
}