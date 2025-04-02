# PIM Matrix Multiplication Compiler

A specialized compiler for transforming C++ matrix multiplication code into the 24-bit instruction format specified by Connolly et al.'s Processing-in-Memory (PIM) architecture. This project was developed for a hackathon that required implementing this specific instruction set architecture (ISA).

## Overview

This compiler translates matrix multiplication algorithms in C++ source code into optimized instruction sequences for execution on a Processing-in-Memory hardware architecture. The implementation generates instructions following the ISA format described in the paper "Flexible Instruction Set Architecture for Programmable Look-up Table based Processing-in-Memory" (Connolly et al., 2021), which is included in this repository.

The project specifically targets the 24-bit instruction format with operation codes (NoOp, PROG, EXE, END) in bits 18-17, core pointer in bits 16-11, read/write flags, and 9-bit address field. While the paper describes a microcoded Controller/Sequencer design with ~120-bit control words, this compiler focuses on generating the appropriate instruction sequences that would be consumed by such a controller.

## Key Features

- **Automatic Pattern Recognition**: Detects different matrix multiplication patterns in source code
- **Enhanced Parser**: Supports multiple implementation formats (classic, vector-based, flattened arrays, custom orderings)
- **Work Distribution**: Parallelizes computation across multiple PIM cores
- **Memory Layout Optimization**: Efficiently arranges matrices in memory for optimized access patterns
- **Instruction Generation**: Produces compact, specialized 24-bit instructions
- **Python-based Simulation**: Validates the generated instructions against NumPy matrix multiplication

## PIM Architecture

### Memory Model

- Memory is organized in rows of 512 elements (`MEMORY_ROW_SIZE`)
- Matrices are arranged linearly with optimized base addresses
- Special handling for matrices that span multiple memory rows

### Instruction Set Architecture (ISA)

- **24-bit Instructions**:
  - Bits 18-17: Operation code (2 bits)
  - Bits 16-11: Core pointer (6 bits)
  - Bit 10: Read flag
  - Bit 9: Write flag
  - Bits 8-0: Address (9 bits)

- **Operation Types**:
  - `00`: NoOp (No Operation)
  - `01`: PROG (Program a core)
  - `10`: EXE (Execute an operation)
  - `11`: END (End operation)

## Compilation Pipeline

1. **Source Code Analysis**: 
   - Parses C/C++ matrix multiplication code
   - Detects matrix dimensions and patterns
   - Identifies variable names for matrices

2. **Three-Address Code (3AC) Generation**:
   - Converts high-level matrix multiplication to intermediate representation

3. **Work Distribution**:
   - Divides the computation across available cores
   - Assigns rows of the result matrix to different cores

4. **Memory Layout Optimization**:
   - Organizes matrices in memory
   - Calculates base addresses and offsets
   - Handles matrices that span multiple memory rows

5. **Instruction Generation**:
   - Generates sequences of 24-bit PIM instructions
   - Produces load, compute, and store operations
   - Implements multiply-accumulate operations

## Supported Matrix Multiplication Patterns

The enhanced parser (`enhanced_parser.cpp`) recognizes several common patterns:

### 1. Classic Triple-Nested Loop

```cpp
for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
        for (int k = 0; k < K; k++) {
            C[i][j] += A[i][k] * B[k][j];
        }
    }
}
```

### 2. Vector-Based Implementation

```cpp
for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
        int sum = 0;
        for (int k = 0; k < K; k++) {
            sum += A[i][k] * B[k][j];
        }
        C[i][j] = sum;
    }
}
```

### 3. Flattened Arrays

```cpp
for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
        int sum = 0;
        for (int k = 0; k < K; k++) {
            sum += A[i*K + k] * B[k*N + j];
        }
        C[i*N + j] = sum;
    }
}
```

### 4. Optimized KIJ Loop Ordering

```cpp
for (int k = 0; k < K; k++) {
    for (int i = 0; i < M; i++) {
        double r = A[i*K + k];
        for (int j = 0; j < N; j++) {
            C[i*N + j] += r * B[k*N + j];
        }
    }
}
```

## Installation

### Prerequisites

The following packages are required to build and run the project:

```bash
apt-get update && apt-get install -y \
    lsb-release \
    build-essential \
    cmake \
    git \
    g++ \
    libgmp-dev \
    libz3-dev \
    libzstd-dev \
    python3 \
    python3-numpy \
    libsystemc \
    libsystemc-dev
```

These dependencies provide:
- Essential build tools and C++ compiler
- CMake build system
- Python3 and NumPy for simulation and validation
- SystemC libraries for hardware architecture simulation
- Support libraries for optimization and high-precision operations

### Building from Source

```bash
# Clone the repository
git clone https://github.com/username/pim-compiler.git
cd pim-compiler

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make -j$(nproc)

# Return to project root
cd ..
```

Or use the provided build script:

```bash
./build.sh
```

## Usage

### Basic Command-Line Usage

```bash
build/pim_compiler <input_file> [options]
```

### Options

- `-o <file>`: Output file (default: output.pim)
- `-M <value>`: Rows in matrix A (overrides value in input file)
- `-N <value>`: Columns in matrix B (overrides value in input file)
- `-K <value>`: Columns in matrix A / Rows in matrix B (overrides value in input file)
- `-c <value>`: Number of cores to use (default: 4)
- `-p <value>`: Parser to use (0=basic, 1=enhanced [default])
- `-h, --help`: Show help message

### Examples

```bash
# Compile with 2 cores
build/pim_compiler examples/matrix_multiply.cpp -c 2

# Specify matrix dimensions and output file
build/pim_compiler examples/matrix_multiply.cpp -M 256 -N 128 -K 64 -o custom_size.pim

# Use the original parser instead of enhanced
build/pim_compiler examples/matrix_multiply.cpp -p 0
```

### Interactive Mode

For a guided compilation process:

```bash
./interactive_run.sh
```

### Running the Simulator

To validate the generated instructions:

```bash
python3 pim_simulator.py output.pim [options]

# Options:
#   --debug           Enable debug output
#   --no-validate     Skip result validation
#   --deterministic   Use deterministic test matrices
#   --seed SEED       Random seed for matrix generation
```

## Testing

The project includes a comprehensive testing framework:

```bash
# Run all tests
./run_tests.sh

# Run specific test categories
./run_tests.sh --parser    # Test parser components
./run_tests.sh --sizes     # Test different matrix sizes
./run_tests.sh --formats   # Test different matrix multiplication formats
```

## Project Structure

```
├── include/
│   └── pim_compiler.h       # Main header file
├── src/
│   ├── main.cpp             # Main compiler driver
│   ├── parser.cpp           # Basic matrix pattern detection
│   ├── enhanced_parser.cpp  # Advanced matrix pattern detection
│   ├── three_address.cpp    # Intermediate representation generator
│   ├── parallelizer.cpp     # Work distribution across cores
│   ├── memory_layout.cpp    # Memory layout optimization
│   ├── isa_generator.cpp    # Instruction generation
│   └── core_sequence.cpp    # Core-specific instruction sequences
├── examples/
│   └── matrix_multiply.cpp  # Example matrix multiplication code
├── test/
│   ├── test_compiler.cpp    # Main compiler tests
│   └── test_enhanced_parser.cpp # Parser-specific tests
├── build.sh                 # Build script
├── run_tests.sh             # Consolidated test script
├── interactive_run.sh       # Interactive compilation script
└── pim_simulator.py         # Simulation and validation tool
```

## Architecture Details

### Memory Layout

The memory layout optimizer divides memory into rows of 512 elements (`MEMORY_ROW_SIZE`) and arranges matrices with:

- Matrix A starting at base address 0
- Matrix B starting after all rows of A
- Matrix C (result) starting after all rows of B

For each matrix row, it calculates:
- How many memory rows it spans
- Base address and offset for each element

### Core Instruction Generation

Each core receives instructions to:

1. Load rows from matrix A
2. For each result element:
   - Clear accumulator
   - For each dot product element:
     - Load element from matrix B
     - Perform multiply-accumulate operation
   - Store result to matrix C

The `core_sequence.cpp` component handles this generation with optimized memory addressing.

### Optimization Techniques

1. **Loop Ordering**: Uses "kij" instead of "ijk" ordering for better cache locality
2. **Specialized Instructions**: Implements multiply-accumulate as a single operation
3. **Parallelization**: Distributes work evenly among cores
4. **Memory Access Patterns**: Optimizes for sequential access where possible

## Performance Considerations

Matrix multiplication performance depends on:

- Matrix dimensions and sparsity
- Number of available cores
- Memory access patterns
- Matrix layout in memory

The compiler optimizes for these factors by:
- Balancing work across cores
- Minimizing memory row transitions
- Using specialized operations

## Limitations and Future Work

Current limitations:

- Supports only dense matrix multiplication
- No specialized handling for sparse matrices
- Limited to static matrix dimensions in source code
- No auto-tuning for optimal core count

Potential enhancements:

- Support for sparse matrix formats
- Dynamic dimension handling
- Auto-tuning for core allocation
- Extended ISA for more operations
- Integration with higher-level frameworks

## License

This project is licensed under the [GPL-3.0 license](https://github.com/Sriram-PR/PIM-Matrix-Multiplication-Compiler/blob/main/LICENSE).

## About the Project

This compiler was developed as part of a hackathon challenge to implement a compiler targeting the specific PIM instruction set architecture described by Connolly et al. The goal was to demonstrate how C/C++ matrix multiplication code can be efficiently translated into specialized PIM instructions to leverage the unique capabilities of Processing-in-Memory hardware. The project showcases the potential of PIM architectures for accelerating data-intensive operations while minimizing the memory bottleneck.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

