#!/bin/bash

# Monitor script for large-scale SUARA tests

echo "========================================================================"
echo "  SUARA 2D LARGE-SCALE TEST MONITOR"
echo "========================================================================"
echo ""

# Check if tests are still running
if pgrep -f "run_large_tests.sh" > /dev/null || pgrep -f "mpirun.*suara_2d" > /dev/null; then
    echo "✓ Tests are currently RUNNING"
    echo ""
else
    echo "⊗ No tests currently running"
    echo ""
fi

# Show completed tests
echo "Completed Test Results:"
echo "------------------------"
cd suara_2d_results 2>/dev/null || { echo "No results directory found"; exit 1; }

for file in suara_2d_results_P*.md; do
    if [ -f "$file" ]; then
        P=$(echo "$file" | grep -oP 'P\K[0-9]+')
        SIZE=$(wc -l < "$file")
        TIMESTAMP=$(ls -l "$file" | awk '{print $6, $7, $8}')
        
        # Check if test passed or failed
        if grep -q "✓ PASS" "$file"; then
            STATUS="✓ PASS"
        elif grep -q "✗ FAIL" "$file"; then
            STATUS="✗ FAIL"
        else
            STATUS="?"
        fi
        
        printf "  P=%-6d  %s  %3d lines  [%s]\n" "$P" "$STATUS" "$SIZE" "$TIMESTAMP"
    fi
done | sort -t= -k2 -n

echo ""

# Show current test progress from log
echo "Current Progress (from log):"
echo "----------------------------"
cd ..
if [ -f "large_test_run.log" ]; then
    tail -30 large_test_run.log | grep -E "(TESTING WITH P|✓ Completed|✗ ERROR|✗ TIMEOUT|Started at)" | tail -10
else
    echo "  No log file found"
fi

echo ""
echo "========================================================================"
echo "To view full log: tail -f large_test_run.log"
echo "To stop tests:    pkill -f run_large_tests.sh"
echo "========================================================================"
