cmake_minimum_required(VERSION 3.10)
project(PIMCompiler VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Define source files
set(SOURCES
    src/main.cpp
    src/parser.cpp
    src/enhanced_parser.cpp
    src/three_address.cpp
    src/parallelizer.cpp
    src/isa_generator.cpp
    src/core_sequence.cpp
    src/memory_layout.cpp
)

# Create executable
add_executable(pim_compiler ${SOURCES})

# Add test executable for the standard compiler
add_executable(test_compiler test/test_compiler.cpp)

# Add source files for the parser tests
set(PARSER_TEST_SOURCES
    src/enhanced_parser.cpp
)

# Add test executable for the enhanced parser
add_executable(test_enhanced_parser test/test_enhanced_parser.cpp ${PARSER_TEST_SOURCES})

# Link test executables
target_link_libraries(test_compiler PRIVATE)
target_link_libraries(test_enhanced_parser PRIVATE)