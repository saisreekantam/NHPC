#!/bin/bash

# Large-scale 2D SUARA Testing Script
# Tests: 128, 256, 512, 1024, 2048, 4096

echo "========================================================================"
echo "  LARGE-SCALE 2D SUARA ALLREDUCE TESTS"
echo "========================================================================"
echo ""
echo "Testing with process counts: 128, 256, 512, 1024, 2048, 4096"
echo "Note: These tests use oversubscription and may take considerable time"
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

# Test with large process counts
PROCESS_COUNTS=(128 256 512 1024 2048 4096)

# Update master summary file
MASTER_FILE="$RESULTS_DIR/master_summary.md"

# Add section for large tests if not already present
if ! grep -q "Large-Scale Tests" "$MASTER_FILE" 2>/dev/null; then
    echo "" >> "$MASTER_FILE"
    echo "## Large-Scale Tests (P ≥ 128)" >> "$MASTER_FILE"
    echo "" >> "$MASTER_FILE"
    echo "| P | √P | Factorizations | Status | Link |" >> "$MASTER_FILE"
    echo "|---|----|--------------|---------|----|" >> "$MASTER_FILE"
fi

START_TIME=$(date +%s)

for P in "${PROCESS_COUNTS[@]}"; do
    SQRT_P=$(echo "sqrt($P)" | bc -l | xargs printf "%.1f")
    
    echo "========================================================================"
    echo "  TESTING WITH P = $P PROCESSES (√P ≈ $SQRT_P)"
    echo "========================================================================"
    echo "  Started at: $(date '+%H:%M:%S')"
    echo ""
    
    TEST_START=$(date +%s)
    
    # Run with oversubscribe to allow more processes than cores
    mpirun --oversubscribe -np $P ./suara_2d_comprehensive
    
    EXIT_CODE=$?
    TEST_END=$(date +%s)
    TEST_DURATION=$((TEST_END - TEST_START))
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo ""
        echo "✓ Completed testing for P=$P (took ${TEST_DURATION}s)"
        
        # Move result file to results directory
        if [ -f "suara_2d_results_P${P}.md" ]; then
            mv "suara_2d_results_P${P}.md" "$RESULTS_DIR/"
            echo "| $P | $SQRT_P | Multiple | ✓ Success (${TEST_DURATION}s) | [Results](suara_2d_results_P${P}.md) |" >> "$MASTER_FILE"
        fi
    else
        echo "✗ ERROR: Execution failed for P=$P (exit code: $EXIT_CODE)"
        echo "| $P | $SQRT_P | - | ✗ Failed | - |" >> "$MASTER_FILE"
    fi
    
    echo ""
    echo "------------------------------------------------------------------------"
    echo ""
    
    # Small delay between tests
    sleep 2
done

END_TIME=$(date +%s)
TOTAL_DURATION=$((END_TIME - START_TIME))
TOTAL_MINUTES=$((TOTAL_DURATION / 60))
TOTAL_SECONDS=$((TOTAL_DURATION % 60))

# Finalize master summary
echo "" >> "$MASTER_FILE"
echo "### Large-Scale Test Statistics" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"
echo "- **Total runtime:** ${TOTAL_MINUTES}m ${TOTAL_SECONDS}s" >> "$MASTER_FILE"
echo "- **Completed:** $(date)" >> "$MASTER_FILE"
echo "" >> "$MASTER_FILE"

echo "========================================================================"
echo "  ALL LARGE-SCALE TESTS COMPLETED"
echo "========================================================================"
echo ""
echo "Total runtime: ${TOTAL_MINUTES} minutes ${TOTAL_SECONDS} seconds"
echo "Results directory: $RESULTS_DIR/"
echo "Master summary: $MASTER_FILE"
echo ""
echo "Results generated:"
ls -lh "$RESULTS_DIR"/suara_2d_results_P{128,256,512,1024,2048,4096}.md 2>/dev/null || echo "  (Check which tests completed successfully)"
echo ""
