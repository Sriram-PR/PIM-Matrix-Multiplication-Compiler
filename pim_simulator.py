#!/usr/bin/env python3
"""
PIM Simulator - Properly fixed version that correctly implements the memory model.
This version fixes the critical memory addressing issues while maintaining
the original memory-based approach.
"""

import re
import numpy as np
import argparse
from typing import List, Dict, Tuple, Optional

# Constants
MEMORY_ROW_SIZE = 512

# Instruction Types
INSTR_NOOP = 0  # 00
INSTR_PROG = 1  # 01
INSTR_EXE = 2   # 10
INSTR_END = 3   # 11

class PIMCore:
    """Represents a single Processing-in-Memory core"""
    
    def __init__(self, core_id: int):
        self.core_id = core_id
        self.active = False
        self.completed = False
        
        # Matrix row tracking
        self.current_a_row = None  # Current row from matrix A
        self.current_i = None      # Current row index being processed
        self.row_range = None      # Range of rows assigned to this core (start, end)
        
        # Matrix element tracking
        self.current_b_value = None
        self.current_k = None
        self.current_j = None
        
        # Computation state
        self.accumulator = 0
        
        # Instruction state
        self.function = None
        self.next_operation = None
        self.addr_register = None
        self.offset_register = None
        
    def reset(self):
        """Reset the core state"""
        self.__init__(self.core_id)
        
    def __repr__(self):
        return f"Core {self.core_id} | Active: {self.active} | Completed: {self.completed}"

class PIMMemory:
    """Simulates PIM memory with matrices stored in rows"""
    
    def __init__(self, matrix_a: np.ndarray, matrix_b: np.ndarray):
        self.M, self.K = matrix_a.shape
        self.K2, self.N = matrix_b.shape
        assert self.K == self.K2, f"Matrix dimensions mismatch: A is {self.M}x{self.K}, B is {self.K2}x{self.N}"
        
        # Calculate total elements and rows needed for each matrix
        self.size_a = self.M * self.K
        self.size_b = self.K * self.N
        self.size_c = self.M * self.N
        
        self.rows_a = (self.size_a + MEMORY_ROW_SIZE - 1) // MEMORY_ROW_SIZE
        self.rows_b = (self.size_b + MEMORY_ROW_SIZE - 1) // MEMORY_ROW_SIZE
        self.rows_c = (self.size_c + MEMORY_ROW_SIZE - 1) // MEMORY_ROW_SIZE
        
        # Base addresses
        self.base_addr_a = 0
        self.base_addr_b = self.rows_a
        self.base_addr_c = self.rows_a + self.rows_b
        
        # Initialize memory as a list of rows
        total_rows = self.rows_a + self.rows_b + self.rows_c
        self.memory = [np.zeros(MEMORY_ROW_SIZE, dtype=np.int32) for _ in range(total_rows)]
        
        # Store matrices A and B in memory
        self._store_matrix(matrix_a, self.base_addr_a, self.K)
        self._store_matrix(matrix_b, self.base_addr_b, self.N)
        
        # Matrix C will be filled during execution
        self.matrix_c = np.zeros((self.M, self.N), dtype=np.int32)
    
    def _store_matrix(self, matrix: np.ndarray, base_addr: int, cols: int):
        """Store a matrix in memory starting at the given base address"""
        rows, _ = matrix.shape
        for i in range(rows):
            for j in range(cols):
                idx = i * cols + j
                row_idx = base_addr + idx // MEMORY_ROW_SIZE
                col_idx = idx % MEMORY_ROW_SIZE
                self.memory[row_idx][col_idx] = matrix[i, j]
    
    def read(self, addr: int, offset: int) -> int:
        """Read a value from memory at the given address and offset"""
        if addr < 0 or addr >= len(self.memory):
            raise ValueError(f"Memory address out of bounds: {addr}")
        if offset < 0 or offset >= MEMORY_ROW_SIZE:
            raise ValueError(f"Memory offset out of bounds: {offset}")
        return self.memory[addr][offset]
    
    def write(self, addr: int, offset: int, value: int):
        """Write a value to memory at the given address and offset"""
        if addr < 0 or addr >= len(self.memory):
            raise ValueError(f"Memory address out of bounds: {addr}")
        if offset < 0 or offset >= MEMORY_ROW_SIZE:
            raise ValueError(f"Memory offset out of bounds: {offset}")
        self.memory[addr][offset] = value
    
    def get_matrix_element(self, matrix: str, row: int, col: int) -> int:
        """
        Get a specific matrix element directly from memory using proper addressing.
        This is the core of the fix - correctly calculating memory addresses for matrix elements.
        """
        # Calculate the linear index and memory location based on matrix type
        if matrix == "A":
            if row < 0 or row >= self.M or col < 0 or col >= self.K:
                raise ValueError(f"A[{row}][{col}] out of bounds")
            linear_idx = row * self.K + col
            mem_addr = self.base_addr_a + linear_idx // MEMORY_ROW_SIZE
            mem_offset = linear_idx % MEMORY_ROW_SIZE
        elif matrix == "B":
            if row < 0 or row >= self.K or col < 0 or col >= self.N:
                raise ValueError(f"B[{row}][{col}] out of bounds")
            linear_idx = row * self.N + col
            mem_addr = self.base_addr_b + linear_idx // MEMORY_ROW_SIZE
            mem_offset = linear_idx % MEMORY_ROW_SIZE
        elif matrix == "C":
            if row < 0 or row >= self.M or col < 0 or col >= self.N:
                raise ValueError(f"C[{row}][{col}] out of bounds")
            linear_idx = row * self.N + col
            mem_addr = self.base_addr_c + linear_idx // MEMORY_ROW_SIZE
            mem_offset = linear_idx % MEMORY_ROW_SIZE
        else:
            raise ValueError(f"Unknown matrix: {matrix}")
        
        # Return the value from memory
        return self.read(mem_addr, mem_offset)
    
    def set_matrix_element(self, matrix: str, row: int, col: int, value: int):
        """Set a specific matrix element in memory using proper addressing"""
        # Calculate the linear index and memory location based on matrix type
        if matrix == "A":
            if row < 0 or row >= self.M or col < 0 or col >= self.K:
                raise ValueError(f"A[{row}][{col}] out of bounds")
            linear_idx = row * self.K + col
            mem_addr = self.base_addr_a + linear_idx // MEMORY_ROW_SIZE
            mem_offset = linear_idx % MEMORY_ROW_SIZE
        elif matrix == "B":
            if row < 0 or row >= self.K or col < 0 or col >= self.N:
                raise ValueError(f"B[{row}][{col}] out of bounds")
            linear_idx = row * self.N + col
            mem_addr = self.base_addr_b + linear_idx // MEMORY_ROW_SIZE
            mem_offset = linear_idx % MEMORY_ROW_SIZE
        elif matrix == "C":
            if row < 0 or row >= self.M or col < 0 or col >= self.N:
                raise ValueError(f"C[{row}][{col}] out of bounds")
            linear_idx = row * self.N + col
            mem_addr = self.base_addr_c + linear_idx // MEMORY_ROW_SIZE
            mem_offset = linear_idx % MEMORY_ROW_SIZE
        else:
            raise ValueError(f"Unknown matrix: {matrix}")
        
        # Write the value to memory
        self.write(mem_addr, mem_offset, value)
    
    def get_matrix_indices(self, addr: int, offset: int) -> Optional[Tuple[int, int, str]]:
        """
        Convert memory address and offset to matrix indices.
        Returns (row_idx, col_idx, matrix_name) or None if not a valid matrix access.
        """
        if addr < self.base_addr_b:
            # Matrix A
            element_idx = (addr - self.base_addr_a) * MEMORY_ROW_SIZE + offset
            if element_idx >= self.size_a:
                return None
            row_idx = element_idx // self.K
            col_idx = element_idx % self.K
            return (row_idx, col_idx, "A")
            
        elif addr < self.base_addr_c:
            # Matrix B
            element_idx = (addr - self.base_addr_b) * MEMORY_ROW_SIZE + offset
            if element_idx >= self.size_b:
                return None
            row_idx = element_idx // self.N
            col_idx = element_idx % self.N
            return (row_idx, col_idx, "B")
            
        else:
            # Matrix C
            element_idx = (addr - self.base_addr_c) * MEMORY_ROW_SIZE + offset
            if element_idx >= self.size_c:
                return None
            row_idx = element_idx // self.N
            col_idx = element_idx % self.N
            return (row_idx, col_idx, "C")
    
    def get_result_matrix(self) -> np.ndarray:
        """Extract the result matrix C from memory"""
        # Reconstitute matrix C from memory
        for i in range(self.M):
            for j in range(self.N):
                self.matrix_c[i, j] = self.get_matrix_element("C", i, j)
        
        return self.matrix_c

class PIMSimulator:
    """Main simulator for PIM instructions"""
    
    def __init__(self, num_cores: int, matrix_a: np.ndarray, matrix_b: np.ndarray, row_assignments: Dict[int, Tuple[int, int]]):
        self.num_cores = num_cores
        self.cores = [PIMCore(i) for i in range(num_cores)]
        self.memory = PIMMemory(matrix_a, matrix_b)
        self.cycle_count = 0
        self.matrix_a = matrix_a
        self.matrix_b = matrix_b
        
        # Set row assignments for each core
        for core_id, (start_row, end_row) in row_assignments.items():
            if core_id < len(self.cores):
                self.cores[core_id].row_range = (start_row, end_row)
                
        # For debugging
        self.debug_enabled = False
        # Track row transitions for debugging
        self.row_transitions = {}  # Maps core_id to list of row indices processed
    
    def debug(self, message: str):
        """Print debug message if debug is enabled"""
        if self.debug_enabled:
            print(f"[DEBUG] {message}")
            
    def enable_debug(self):
        """Enable debug output"""
        self.debug_enabled = True
    
    def parse_instruction(self, instr_hex: str) -> Tuple[int, int, bool, bool, int]:
        """Parse a hex instruction into its components"""
        # Convert hex to int
        instr = int(instr_hex, 16)
        
        # Extract fields
        instr_type = (instr >> 17) & 0x3        # Bits 18-17 (opcode)
        core_ptr = (instr >> 11) & 0x3F         # Bits 16-11 (core pointer)
        read_flag = ((instr >> 10) & 0x1) != 0  # Bit 10 (read flag)
        write_flag = ((instr >> 9) & 0x1) != 0  # Bit 9 (write flag)
        addr = instr & 0x1FF                    # Bits 8-0 (address)
        
        return instr_type, core_ptr, read_flag, write_flag, addr
    
    def execute_instruction(self, instr_hex: str) -> bool:
        """Execute a single PIM instruction"""
        if not instr_hex or instr_hex.startswith('#'):
            return True  # Skip comments and empty lines
        
        instr_type, core_ptr, read_flag, write_flag, addr = self.parse_instruction(instr_hex)
        
        # Check if core_ptr is valid
        if core_ptr >= self.num_cores:
            print(f"Warning: Instruction references core {core_ptr} but only {self.num_cores} cores are available")
            return True
        
        core = self.cores[core_ptr]
        
        # Process instruction based on type
        if instr_type == INSTR_NOOP:
            self.debug(f"Core {core_ptr}: NOOP")
            # No operation
            
        elif instr_type == INSTR_PROG:
            self.debug(f"Core {core_ptr}: PROG func={addr} read={read_flag} write={write_flag}")
            # Program the core
            core.active = True
            core.function = addr
            core.completed = False
            
        elif instr_type == INSTR_EXE:
            if not core.active:
                print(f"Warning: Executing on inactive core {core_ptr}")
                return True
                
            self.debug(f"Core {core_ptr}: EXE addr={addr} read={read_flag} write={write_flag}")
            
            if read_flag:
                # Load operation - First part: set address
                core.addr_register = addr
                core.next_operation = "read"
                
            elif write_flag:
                # Store operation - First part: set address
                core.addr_register = addr
                core.next_operation = "write"
                
            else:
                # Execute a specific operation or set offset
                if core.next_operation == "read":
                    # Second part of load: set offset and perform read
                    offset = addr
                    mem_addr = core.addr_register
                    
                    # Get matrix indices (if applicable)
                    indices = self.memory.get_matrix_indices(mem_addr, offset)
                    
                    # Read the value from memory
                    value = self.memory.read(mem_addr, offset)
                    
                    if indices and indices[2] == "A":
                        # Reading from matrix A
                        row_idx, col_idx, _ = indices
                        
                        # Update row tracking if this is a row access (usually first element)
                        if col_idx == 0:  # First element of the row
                            if core.current_i != row_idx:
                                self.debug(f"Core {core_ptr}: Reading from matrix A, setting row index to {row_idx}")
                                core.current_i = row_idx
                                
                                # Track row transitions
                                if core_ptr not in self.row_transitions:
                                    self.row_transitions[core_ptr] = []
                                if row_idx not in self.row_transitions[core_ptr]:
                                    self.row_transitions[core_ptr].append(row_idx)
                        
                        self.debug(f"Core {core_ptr}: Read A[{row_idx}][{col_idx}] = {value}")
                        
                    elif indices and indices[2] == "B":
                        # Reading from matrix B
                        row_idx, col_idx, _ = indices
                        
                        # Store B matrix info for MAC operation
                        core.current_k = row_idx     # Row in B is column in A for dot product
                        core.current_j = col_idx     # Column in B will be column in output
                        core.current_b_value = value # Store the B value for MAC
                        
                        self.debug(f"Core {core_ptr}: Read B[{row_idx}][{col_idx}] = {value}")
                    
                    core.next_operation = None
                    
                elif core.next_operation == "write":
                    # Second part of store: set offset and perform write
                    offset = addr
                    mem_addr = core.addr_register
                    
                    # Get matrix indices (if applicable)
                    indices = self.memory.get_matrix_indices(mem_addr, offset)
                    
                    if indices and indices[2] == "C":
                        # Writing to matrix C
                        row_idx, col_idx, _ = indices
                        
                        # Update row tracking
                        if core.current_i != row_idx:
                            self.debug(f"Core {core_ptr}: Writing to C, updating row index from {core.current_i} to {row_idx}")
                            core.current_i = row_idx
                            
                            # Track row transitions
                            if core_ptr not in self.row_transitions:
                                self.row_transitions[core_ptr] = []
                            if row_idx not in self.row_transitions[core_ptr]:
                                self.row_transitions[core_ptr].append(row_idx)
                        
                        # Write accumulator to memory
                        self.memory.write(mem_addr, offset, core.accumulator)
                        self.debug(f"Core {core_ptr}: Write {core.accumulator} to C[{row_idx}][{col_idx}]")
                    else:
                        # Generic memory write
                        self.memory.write(mem_addr, offset, core.accumulator)
                        self.debug(f"Core {core_ptr}: Write {core.accumulator} to memory addr={mem_addr} offset={offset}")
                    
                    core.next_operation = None
                    
                else:
                    # Special operations
                    if addr == 0:
                        # Clear accumulator
                        core.accumulator = 0
                        self.debug(f"Core {core_ptr}: Clear accumulator")
                        
                    elif addr == 2:
                        # Multiply-accumulate - THE CORE FIX IS HERE
                        # This is now using proper memory addressing to get the A value
                        if core.current_i is not None and core.current_k is not None and core.current_b_value is not None:
                            try:
                                # Get the A value directly from memory with proper addressing
                                a_value = self.memory.get_matrix_element("A", core.current_i, core.current_k)
                                
                                # Proper multiply-accumulate
                                product = a_value * core.current_b_value
                                core.accumulator += product
                                
                                self.debug(f"Core {core_ptr}: MAC: {core.accumulator} += A[{core.current_i}][{core.current_k}]({a_value}) * B[{core.current_k}][{core.current_j}]({core.current_b_value}) = {product}")
                            except ValueError as e:
                                self.debug(f"Core {core_ptr}: MAC: {e}")
                        else:
                            self.debug(f"Core {core_ptr}: MAC: Missing matrix indices, skipping")
                                    
        elif instr_type == INSTR_END:
            self.debug(f"Core {core_ptr}: END")
            # End operation on this core
            core.active = False
            core.completed = True
            
        else:
            print(f"Warning: Unknown instruction type {instr_type}")
            
        self.cycle_count += 1
        return True
    
    def execute_program(self, instructions: List[str]) -> np.ndarray:
        """Execute a sequence of PIM instructions"""
        self.cycle_count = 0
        self.row_transitions = {}
        
        # Initialize all cores with their row ranges
        for core in self.cores:
            if hasattr(core, 'row_range') and core.row_range is not None:
                # Set initial row to the start of the range
                core.current_i = core.row_range[0]
                self.debug(f"Initializing Core {core.core_id} to process row {core.current_i}")
                
                # Initialize row transitions tracking
                self.row_transitions[core.core_id] = [core.current_i]
            else:
                # Default to the core's ID if no range is assigned
                core.current_i = core.core_id
                self.debug(f"No row range for Core {core.core_id}, using core_id as row: {core.current_i}")
                
                # Initialize row transitions tracking
                self.row_transitions[core.core_id] = [core.current_i]
        
        # Pre-process instructions to identify explicit row transitions
        explicit_row_transitions = {}  # Maps instruction index to (core_id, row_idx)
        for i, instr in enumerate(instructions):
            if "Processing row" in instr:
                match = re.search(r"Core (\d+).*Processing row (\d+)", instr)
                if match:
                    core_id, row_idx = map(int, match.groups())
                    explicit_row_transitions[i] = (core_id, int(row_idx))
        
        # Execute instructions
        for i, instr in enumerate(instructions):
            # Check if we need to update any core's row BEFORE executing this instruction
            if i in explicit_row_transitions:
                core_id, row_idx = explicit_row_transitions[i]
                if core_id < len(self.cores):
                    # Update the core's current row BEFORE executing the next instruction
                    if self.cores[core_id].current_i != row_idx:
                        self.debug(f"Core {core_id} explicitly switching from row {self.cores[core_id].current_i} to row {row_idx}")
                        self.cores[core_id].current_i = row_idx
                        
                        # Track row transitions
                        if core_id not in self.row_transitions:
                            self.row_transitions[core_id] = []
                        if row_idx not in self.row_transitions[core_id]:
                            self.row_transitions[core_id].append(row_idx)
            
            # Skip comments and empty lines
            if not instr or instr.startswith('#'):
                continue
                
            # Extract and execute the instruction
            instr_parts = instr.split('#', 1)
            instr_hex = instr_parts[0].strip()
            
            if not instr_hex:
                continue
                
            success = self.execute_instruction(instr_hex)
            if not success:
                print(f"Execution failed at instruction {i}: {instr}")
                break
        
        # Debug output - summarize row transitions
        if self.debug_enabled:
            for core_id, rows in self.row_transitions.items():
                self.debug(f"Core {core_id} processed rows: {sorted(rows)}")
        
        # Check if all cores have completed
        all_completed = all(core.completed for core in self.cores)
        if not all_completed:
            incomplete_cores = [core.core_id for core in self.cores if not core.completed]
            print(f"Warning: Not all cores completed. Incomplete cores: {incomplete_cores}")
        
        # Get the result matrix
        result = self.memory.get_result_matrix()
        
        print(f"Execution completed in {self.cycle_count} cycles")
        return result
        
    def validate_result(self, pim_result: np.ndarray) -> bool:
        """Validate the PIM result against a direct numpy matrix multiplication"""
        expected = np.matmul(self.matrix_a, self.matrix_b)
        
        # Check if shapes match
        if pim_result.shape != expected.shape:
            print(f"Shape mismatch: PIM result {pim_result.shape}, expected {expected.shape}")
            return False
            
        # For large matrices, show a compact summary of differences
        if pim_result.shape[0] > 8:
            # Calculate differences
            diff = pim_result - expected
            diff_count = np.count_nonzero(diff)
            total_elements = np.prod(pim_result.shape)
            error_percent = (diff_count / total_elements) * 100
            
            # Check if results match within tolerance
            max_diff = np.max(np.abs(diff)) if diff_count > 0 else 0
            matches = diff_count == 0
            
            if matches:
                print("Result validation PASSED!")
            else:
                print(f"Result validation FAILED: {diff_count}/{total_elements} elements differ ({error_percent:.2f}%)")
                print(f"Maximum difference: {max_diff}")
                
                # Show first few rows only to avoid cluttering output
                print("\nExpected (first 4 rows):")
                print(expected[:4])
                print("\nPIM Result (first 4 rows):")
                print(pim_result[:4])
                print("\nDifferences (first 4 rows):")
                print(diff[:4])
                
                # Show a few rows from the middle if available
                if pim_result.shape[0] > 8:
                    mid = pim_result.shape[0] // 2
                    print(f"\nMiddle rows ({mid}-{mid+3}):")
                    print("Expected:")
                    print(expected[mid:mid+4])
                    print("PIM Result:")
                    print(pim_result[mid:mid+4])
                    print("Differences:")
                    print(diff[mid:mid+4])
                
                # Show last few rows
                print(f"\nLast rows ({pim_result.shape[0]-4}-{pim_result.shape[0]-1}):")
                print("Expected:")
                print(expected[-4:])
                print("PIM Result:")
                print(pim_result[-4:])
                print("Differences:")
                print(diff[-4:])
        else:
            # For small matrices, show the full comparison
            correct = np.array_equal(pim_result, expected)
            
            if not correct:
                print("Result validation FAILED!")
                print("Expected:")
                print(expected)
                print("PIM Result:")
                print(pim_result)
                print("Difference:")
                print(pim_result - expected)
            else:
                print("Result validation PASSED!")
                
        return np.array_equal(pim_result, expected)

def parse_input_file(filename: str) -> Tuple[int, int, int, int, List[str], Dict[int, Tuple[int, int]]]:
    """
    Parse the input file to extract matrix dimensions, number of cores, and row assignments.
    Returns (M, K, N, num_cores, instructions, row_assignments) where row_assignments
    is a dictionary mapping core_id to (start_row, end_row).
    """
    instructions = []
    dimensions = None
    num_cores = None
    row_assignments = {}
    
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            
            # Extract matrix dimensions
            if "Matrix dimensions:" in line:
                # Format: # Matrix dimensions: MxK * KxN
                match = re.search(r"(\d+)x(\d+) \* (\d+)x(\d+)", line)
                if match:
                    M, K1, K2, N = map(int, match.groups())
                    if K1 != K2:
                        print(f"Warning: Matrix dimensions mismatch: {K1} != {K2}")
                    dimensions = (M, K1, N)
            
            # Extract number of cores
            elif "Using" in line and "cores" in line:
                # Format: # Using X cores
                match = re.search(r"Using (\d+) cores", line)
                if match:
                    num_cores = int(match.group(1))
            
            # Extract row assignments for each core
            elif "Core" in line and "Rows" in line:
                # Format: # Instructions for Core X (Rows Y to Z)
                match = re.search(r"Core (\d+).*Rows (\d+) to (\d+)", line)
                if match:
                    core_id, start_row, end_row = map(int, match.groups())
                    row_assignments[core_id] = (start_row, end_row)
            
            # Add the instruction (including comments for readability)
            instructions.append(line)
    
    if dimensions is None:
        raise ValueError("Matrix dimensions not found in input file")
    
    if num_cores is None:
        raise ValueError("Number of cores not found in input file")
    
    if not row_assignments:
        # If no row assignments were found, distribute rows evenly
        M, K, N = dimensions
        rows_per_core = (M + num_cores - 1) // num_cores  # Ceiling division
        for core_id in range(num_cores):
            start_row = core_id * rows_per_core
            end_row = min(start_row + rows_per_core - 1, M - 1)
            if start_row <= end_row:  # Only assign if there's work to do
                row_assignments[core_id] = (start_row, end_row)
    
    M, K, N = dimensions
    return M, K, N, num_cores, instructions, row_assignments

def generate_test_matrices(M: int, K: int, N: int, random=True, seed=None) -> Tuple[np.ndarray, np.ndarray]:
    """Generate test matrices for simulation"""
    if seed is not None:
        np.random.seed(seed)
        
    if random:
        # Generate random integer matrices
        # Using small integer values (-10 to 10) for easier debugging
        A = np.random.randint(-10, 11, (M, K), dtype=np.int32)
        B = np.random.randint(-10, 11, (K, N), dtype=np.int32)
    else:
        # Use deterministic patterns
        # Matrix A: each element is i+j
        A = np.zeros((M, K), dtype=np.int32)
        for i in range(M):
            for j in range(K):
                A[i, j] = i + j
        
        # Matrix B: each element is i-j
        B = np.zeros((K, N), dtype=np.int32)
        for i in range(K):
            for j in range(N):
                B[i, j] = i - j
    
    return A, B

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='PIM Simulator for Matrix Multiplication')
    parser.add_argument('input_file', help='Input file with PIM instructions')
    parser.add_argument('--debug', action='store_true', help='Enable debug output')
    parser.add_argument('--no-validate', action='store_true', help='Skip result validation')
    parser.add_argument('--deterministic', action='store_true', help='Use deterministic test matrices instead of random')
    parser.add_argument('--seed', type=int, help='Random seed for matrix generation')
    args = parser.parse_args()
    
    # Parse input file
    try:
        M, K, N, num_cores, instructions, row_assignments = parse_input_file(args.input_file)
        print(f"Parsed matrix dimensions: {M}x{K} * {K}x{N}")
        print(f"Using {num_cores} cores")
        
        # Generate test matrices
        random = not args.deterministic
        A, B = generate_test_matrices(M, K, N, random=random, seed=args.seed)
        
        print("\nMatrix A:")
        print(A)
        print("\nMatrix B:")
        print(B)
        
        # Create and initialize simulator
        simulator = PIMSimulator(num_cores, A, B, row_assignments)
        if args.debug:
            simulator.enable_debug()
        
        # Execute program
        print("\nExecuting PIM instructions...")
        result = simulator.execute_program(instructions)
        
        print("\nResult Matrix C:")
        print(result)
        
        # Validate result
        if not args.no_validate:
            print("\nValidating result...")
            simulator.validate_result(result)
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())