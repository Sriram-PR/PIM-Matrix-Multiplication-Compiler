#include "pim_compiler.h"
#include <algorithm>
#include <iostream>
#include <cmath> // For std::ceil

std::vector<WorkAssignment> distributeWork(const MatrixDimensions& dims, int numCores) {
    std::vector<WorkAssignment> assignments;
    
    // Check if there are more cores than matrix rows
    if (numCores > dims.M) {
        std::cout << "Warning: More cores (" << numCores << ") than matrix rows ("
                  << dims.M << "). Using only " << dims.M << " cores." << std::endl;
        numCores = dims.M;
    }
    
    // Calculate rows per core (divide rows evenly among cores)
    int rowsPerCore = std::ceil(static_cast<double>(dims.M) / numCores);
    
    std::cout << "Distributing work: " << dims.M << " rows across " << numCores 
              << " cores (approx. " << rowsPerCore << " rows per core)" << std::endl;
    
    // Assign rows to each core
    for (int core = 0; core < numCores; core++) {
        WorkAssignment work;
        work.coreId = core;
        work.startRow = core * rowsPerCore;
        work.endRow = std::min((core + 1) * rowsPerCore - 1, dims.M - 1);
        
        // Only add if this core actually has work to do
        if (work.startRow <= work.endRow) {
            assignments.push_back(work);
            std::cout << "  Core " << core << ": Rows " << work.startRow << " to " 
                      << work.endRow << " (" << (work.endRow - work.startRow + 1) 
                      << " rows)" << std::endl;
        }
    }
    
    return assignments;
}