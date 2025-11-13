#!/bin/bash

# Streamlined 3D SUARA Testing Script
# Tests only critical configurations for laptop execution

echo "========================================================================"
echo "  STREAMLINED 3D SUARA ALLREDUCE TESTING"
echo "========================================================================"
echo ""

# Compile
echo "Compiling suara_3d_streamlined.c..."
mpicc -o suara_3d_streamlined suara_3d_streamlined.c -lm -O3
if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed!"
    exit 1
fi
echo "✓ Compilation successful"
echo ""

# Test with perfect cube process counts (laptop-friendly)
PROCESS_COUNTS=(8 27 64)

echo "Testing with perfect cube process counts only:"
echo "  P=8   → 2×2×2 grid"
echo "  P=27  → 3×3×3 grid"  
echo "  P=64  → 4×4×4 grid"
echo ""

for P in "${PROCESS_COUNTS[@]}"; do
    echo "========================================================================"
    echo "  TESTING WITH P = $P PROCESSES"
    echo "========================================================================"
    
    mpirun -np $P ./suara_3d_streamlined
    
    if [ $? -ne 0 ]; then
        echo "ERROR: Execution failed for P=$P"
        echo "Note: Make sure you have at least $P cores available"
    else
        echo ""
        echo "✓ Completed testing for P=$P"
    fi
    
    echo ""
    echo "------------------------------------------------------------------------"
    echo ""
done

echo "========================================================================"
echo "  TESTING COMPLETED"
echo "========================================================================"
echo ""
echo "Key findings to look for:"
echo "  1. Diminishing returns: 3D speedup modest compared to 2D"
echo "  2. Grid independence: Rab³ similar times across configurations"
echo "  3. Symmetric preference: Ring³ best at (∛P)³ grids"
echo "  4. All tests should show ✓ PASS for correctness"
echo ""
