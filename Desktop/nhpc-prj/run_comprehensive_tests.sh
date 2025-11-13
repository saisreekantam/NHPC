#!/bin/bash

# Comprehensive 2D SUARA Testing Script
# Tests all algorithm combinations across multiple process counts

echo "========================================================================"
echo "  COMPREHENSIVE 2D SUARA ALLREDUCE TESTING"
echo "========================================================================"
echo ""

# Compile the code
echo "Compiling suara_2d_comprehensive.c..."
mpicc -o suara_2d_comprehensive suara_2d_comprehensive.c -lm -O3
if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed!"
    exit 1
fi
echo "✓ Compilation successful"
echo ""

# Test with different process counts
# These are chosen to have multiple factorizations
PROCESS_COUNTS=(16 36 64 100 144)

for P in "${PROCESS_COUNTS[@]}"; do
    echo "========================================================================"
    echo "  TESTING WITH P = $P PROCESSES"
    echo "========================================================================"
    
    # Check if we have enough processes
    # Run with the specified number of processes
    mpirun -np $P ./suara_2d_comprehensive
    
    if [ $? -ne 0 ]; then
        echo "ERROR: Execution failed for P=$P"
    else
        echo ""
        echo "✓ Completed testing for P=$P"
        echo ""
    fi
    
    echo ""
    echo "------------------------------------------------------------------------"
    echo ""
done

echo "========================================================================"
echo "  ALL TESTS COMPLETED"
echo "========================================================================"
echo ""
echo "Summary of tests run:"
for P in "${PROCESS_COUNTS[@]}"; do
    echo "  - P = $P processes"
done
echo ""
echo "Results saved above. Key findings:"
echo "  1. Compare symmetric vs asymmetric configurations"
echo "  2. Observe which algorithm pairs perform best"
echo "  3. Validate mathematical predictions"
echo "  4. Check for correctness across all configurations"
echo ""
