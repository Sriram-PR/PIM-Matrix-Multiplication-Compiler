#include <iostream>
#include <vector>

// This is an implementation of matrix multiplication
// that uses custom naming and different patterns

// Constants for matrix sizes
const int MATRIX_SIZE_A_ROWS = 256;
const int MATRIX_SIZE_A_COLS = 128; 
const int MATRIX_SIZE_B_ROWS = 128;  // Must match A_COLS
const int MATRIX_SIZE_B_COLS = 64;

// Function prototype
void compute_matrix_product(
    double* matrix_input_first,   // First input matrix (A)
    double* matrix_input_second,  // Second input matrix (B) 
    double* matrix_output         // Output matrix (C)
);

int main() {
    // Allocate memory for matrices
    double* first_matrix = new double[MATRIX_SIZE_A_ROWS * MATRIX_SIZE_A_COLS];
    double* second_matrix = new double[MATRIX_SIZE_B_ROWS * MATRIX_SIZE_B_COLS];
    double* result_matrix = new double[MATRIX_SIZE_A_ROWS * MATRIX_SIZE_B_COLS];
    
    // Initialize matrices with some values
    for (int i = 0; i < MATRIX_SIZE_A_ROWS; i++) {
        for (int j = 0; j < MATRIX_SIZE_A_COLS; j++) {
            first_matrix[i * MATRIX_SIZE_A_COLS + j] = i + j;
        }
    }
    
    for (int i = 0; i < MATRIX_SIZE_B_ROWS; i++) {
        for (int j = 0; j < MATRIX_SIZE_B_COLS; j++) {
            second_matrix[i * MATRIX_SIZE_B_COLS + j] = i - j;
        }
    }
    
    // Zero initialize result matrix
    for (int i = 0; i < MATRIX_SIZE_A_ROWS; i++) {
        for (int j = 0; j < MATRIX_SIZE_B_COLS; j++) {
            result_matrix[i * MATRIX_SIZE_B_COLS + j] = 0.0;
        }
    }
    
    // Perform matrix multiplication
    compute_matrix_product(first_matrix, second_matrix, result_matrix);
    
    // Cleanup
    delete[] first_matrix;
    delete[] second_matrix;
    delete[] result_matrix;
    
    return 0;
}

// Implementation of matrix multiplication with atypical loop ordering
void compute_matrix_product(
    double* matrix_input_first,  
    double* matrix_input_second, 
    double* matrix_output
) {
    // Unconventional loop ordering (kij instead of ijk)
    for (int k = 0; k < MATRIX_SIZE_A_COLS; k++) {
        for (int i = 0; i < MATRIX_SIZE_A_ROWS; i++) {
            double r = matrix_input_first[i * MATRIX_SIZE_A_COLS + k];
            for (int j = 0; j < MATRIX_SIZE_B_COLS; j++) {
                matrix_output[i * MATRIX_SIZE_B_COLS + j] += 
                    r * matrix_input_second[k * MATRIX_SIZE_B_COLS + j];
            }
        }
    }
}