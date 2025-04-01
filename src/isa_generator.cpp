#include "pim_compiler.h"
#include <cstdint>
#include <iomanip>
#include <sstream>

// Helper function to convert a 24-bit value to a 6-character hex string
std::string to_hex_string(int value) {
    std::stringstream ss;
    ss << std::hex << std::setw(6) << std::setfill('0') << (value & 0xFFFFFF);
    return ss.str();
}

// Generate a NoOp instruction
std::string genNoOpInstr() {
    // Instruction format: 00 in bits 18-17 (NoOp)
    uint32_t instruction = 0;
    
    // Build instruction - NoOp is 00 in bits 18-17
    instruction |= (0 << 17);  // Type: NoOp = 00 at bits 18-17
    
    // Convert to hex string
    return to_hex_string(instruction);
}

// Generate a PROG instruction - used to program a core
std::string genProgInstr(int coreId, bool read, bool write, int addr) {
    // Instruction format: 01 in bits 18-17 (PROG)
    uint32_t instruction = 0;
    
    // Ensure values are within valid ranges
    coreId &= 0x3F;     // 6 bits for core pointer
    addr &= 0x1FF;      // 9 bits for address
    
    // Build instruction
    instruction |= (1 << 17);                // Type: PROG = 01 (bit 17 set, bit 18 unset)
    instruction |= (coreId << 11);           // 6-bit pointer field at bits 16-11
    instruction |= ((read ? 1 : 0) << 10);   // Read bit at bit 10
    instruction |= ((write ? 1 : 0) << 9);   // Write bit at bit 9
    instruction |= addr;                     // 9-bit address at bits 8-0
    
    // Convert to hex string
    return to_hex_string(instruction);
}

// Generate an EXE instruction - used to execute an operation
std::string genExeInstr(int coreId, bool read, bool write, int addr) {
    // Instruction format: 10 in bits 18-17 (EXE)
    uint32_t instruction = 0;
    
    // Ensure values are within valid ranges
    coreId &= 0x3F;     // 6 bits for core pointer
    addr &= 0x1FF;      // 9 bits for address
    
    // Build instruction
    instruction |= (2 << 17);                // Type: EXE = 10 (bit 18 set, bit 17 unset)
    instruction |= (coreId << 11);           // 6-bit pointer field at bits 16-11
    instruction |= ((read ? 1 : 0) << 10);   // Read bit at bit 10
    instruction |= ((write ? 1 : 0) << 9);   // Write bit at bit 9
    instruction |= addr;                     // 9-bit address at bits 8-0
    
    // Convert to hex string
    return to_hex_string(instruction);
}

// Generate an END instruction - terminates an operation
std::string genEndInstr(int coreId, bool read, bool write, int addr) {
    // Instruction format: 11 in bits 18-17 (END)
    uint32_t instruction = 0;
    
    // Ensure values are within valid ranges
    coreId &= 0x3F;     // 6 bits for core pointer
    addr &= 0x1FF;      // 9 bits for address
    
    // Build instruction
    instruction |= (3 << 17);                // Type: END = 11 (both bits 17 and 18 set)
    instruction |= (coreId << 11);           // 6-bit pointer field at bits 16-11
    instruction |= ((read ? 1 : 0) << 10);   // Read bit at bit 10
    instruction |= ((write ? 1 : 0) << 9);   // Write bit at bit 9
    instruction |= addr;                     // 9-bit address at bits 8-0
    
    // Convert to hex string
    return to_hex_string(instruction);
}