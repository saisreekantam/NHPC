#!/bin/bash

# Comprehensive 2D SUARA Testing Script for Multiple Process Counts
# Tests with process counts: 4, 16, 36, 64, 100, 144, 256, 400, 576, 1024

echo "========================================================================"
echo "  COMPREHENSIVE 2D SUARA ALLREDUCE TESTING - MULTIPLE PROCESS COUNTS"
echo "========================================================================"
echo ""
echo "This will run tests with various process counts and save results"
echo "to individual markdown files."
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

# Test with different process counts
# Using powers and interesting factorizations
PROCESS_COUNTS=(4 16 36 64 100 144 256 400 576 1024)

# Create a master summary file
MASTER_FILE="$RESULTS_DIR/master_summary.md"
echo "# SUARA 2D Comprehensive Test Results" > "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "**Generated:** $(date)" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "## Test Configuration" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "- **Message size:** ~1M integers (~4 MB)" >> "$MASTER_FILE"
echo "- **Warmup iterations:** 2" >> "$MASTER_FILE"
echo "- **Timing iterations:** 10" >> "$MASTER_FILE"
echo "- **Algorithms tested:** Ring, Rabenseifner" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "## Summary of All Tests" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "| P | Status | Best Configuration | Time (s) | Speedup | Details |" >> "$MASTER_FILE"
echo "|---|--------|-------------------|----------|---------|---------|" >> "$MASTER_FILE"

for P in "${PROCESS_COUNTS[@]}"; do
    echo "========================================================================"
    echo "  TESTING WITH P = $P PROCESSES"
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
            
            # Extract best result for master summary (simple grep approach)
            BEST_INFO=$(grep -A 1 "### Best Configuration" "$RESULTS_DIR/suara_2d_results_P${P}.md" | tail -1)
            echo "| $P | ✓ Success | See details | - | - | [P=$P results](suara_2d_results_P${P}.md) |" >> "$MASTER_FILE"
        fi
    else
        echo "✗ ERROR: Execution failed for P=$P"
        echo "| $P | ✗ Failed | - | - | - | - |" >> "$MASTER_FILE"
    fi
    
    echo ""
    echo "------------------------------------------------------------------------"
    echo ""
    
    # Small delay to avoid overwhelming the system
    sleep 1
done

# Finalize master summary
echo "" >> "$MASTER_FILE"
echo "## Process Count Analysis" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "### Perfect Squares" >> "$MASTER_FILE"
echo "- P = 4 (2×2)" >> "$MASTER_FILE"
echo "- P = 16 (4×4)" >> "$MASTER_FILE"
echo "- P = 36 (6×6)" >> "$MASTER_FILE"
echo "- P = 64 (8×8)" >> "$MASTER_FILE"
echo "- P = 100 (10×10)" >> "$MASTER_FILE"
echo "- P = 144 (12×12)" >> "$MASTER_FILE"
echo "- P = 256 (16×16)" >> "$MASTER_FILE"
echo "- P = 400 (20×20)" >> "$MASTER_FILE"
echo "- P = 576 (24×24)" >> "$MASTER_FILE"
echo "- P = 1024 (32×32)" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "### Key Observations" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "All tested process counts are perfect squares, allowing for symmetric" >> "$MASTER_FILE"
echo "√P × √P configurations as well as various asymmetric factorizations." >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "---" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "*Test completed: $(date)*" >> "$MASTER_FILE"

echo "========================================================================"
echo "  ALL TESTS COMPLETED"
echo "========================================================================"
echo ""
echo "Results saved to: $RESULTS_DIR/"
echo "Master summary: $MASTER_FILE"
echo ""
echo "Individual result files:"
ls -lh "$RESULTS_DIR/"/*.md
echo ""
