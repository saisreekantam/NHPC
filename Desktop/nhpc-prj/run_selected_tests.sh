#!/bin/bash

# Lightweight 2D SUARA Testing Script - Selected Process Counts
# Tests: 4, 16, 36, 64, 100, 144, 256

echo "========================================================================"
echo "  SELECTED 2D SUARA ALLREDUCE TESTS"
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

# Create results directory
RESULTS_DIR="suara_2d_results"
mkdir -p "$RESULTS_DIR"

# Test with selected process counts (reasonable for local testing)
PROCESS_COUNTS=(4 16 36 64 100 144 256)

# Create a master summary file
MASTER_FILE="$RESULTS_DIR/master_summary.md"
echo "# SUARA 2D Selected Test Results" > "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "**Generated:** $(date)" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "## Summary of Tests" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "| P | √P | Factorizations | Status | Link |" >> "$MASTER_FILE"
echo "|---|----|--------------|---------|----|" >> "$MASTER_FILE"

for P in "${PROCESS_COUNTS[@]}"; do
    SQRT_P=$(echo "sqrt($P)" | bc -l | xargs printf "%.1f")
    
    echo "========================================================================"
    echo "  TESTING WITH P = $P PROCESSES (√P ≈ $SQRT_P)"
    echo "========================================================================"
    echo ""
    
    # Run with oversubscribe to allow more processes than cores
    mpirun --oversubscribe -np $P ./suara_2d_comprehensive
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✓ Completed testing for P=$P"
        
        # Move result file to results directory
        if [ -f "suara_2d_results_P${P}.md" ]; then
            mv "suara_2d_results_P${P}.md" "$RESULTS_DIR/"
            
            # Count factorizations
            NUM_FACT=$(grep -c "^|" "$RESULTS_DIR/suara_2d_results_P${P}.md" | head -1)
            echo "| $P | $SQRT_P | Multiple | ✓ Success | [Results](suara_2d_results_P${P}.md) |" >> "$MASTER_FILE"
        fi
    else
        echo "✗ ERROR: Execution failed for P=$P"
        echo "| $P | $SQRT_P | - | ✗ Failed | - |" >> "$MASTER_FILE"
    fi
    
    echo ""
    echo "------------------------------------------------------------------------"
    echo ""
    
    # Small delay
    sleep 1
done

# Finalize master summary
echo "" >> "$MASTER_FILE"
echo "## Notes" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "- All tested process counts are perfect squares" >> "$MASTER_FILE"
echo "- Each test includes symmetric (√P × √P) and asymmetric factorizations" >> "$MASTER_FILE"
echo "- Four algorithm combinations tested: Ring+Ring, Ring+Rab, Rab+Ring, Rab+Rab" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "---" >> "$MASTER_FILE"
echo "*Completed: $(date)*" >> "$MASTER_FILE"

echo "========================================================================"
echo "  ALL TESTS COMPLETED"
echo "========================================================================"
echo ""
echo "Results directory: $RESULTS_DIR/"
echo "Master summary: $MASTER_FILE"
echo ""
