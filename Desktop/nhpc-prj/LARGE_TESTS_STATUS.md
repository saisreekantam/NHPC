# SUARA 2D Large-Scale Testing - Status and Instructions

## Current Status

**Tests Started:** Check `large_test_run.log` for exact time  
**Process Counts Being Tested:** 128, 256, 512, 1024, 2048, 4096

## What's Happening

The large-scale tests are running in the background with process counts from 128 to 4096. Each test:
1. Creates a 2D process grid with multiple factorizations
2. Tests 4 algorithm combinations (Ring+Ring, Ring+Rab, Rab+Ring, Rab+Rab)
3. Runs performance benchmarks and correctness verification
4. Saves results to individual markdown files

## Expected Timeline

Based on P=128 taking ~107 seconds, rough estimates:
- **P=128**: ~2 minutes ✓
- **P=256**: ~4-8 minutes (in progress)
- **P=512**: ~10-20 minutes
- **P=1024**: ~30-60 minutes
- **P=2048**: ~1-2 hours (may fail due to resource limits)
- **P=4096**: ~2-4 hours (may fail due to resource limits)

**Total expected time: 2-7 hours** (depending on system load and failures)

## How to Monitor Progress

### Check if tests are running:
```bash
ps aux | grep -E "(mpirun|suara_2d)" | grep -v grep
```

### Monitor real-time progress:
```bash
tail -f large_test_run.log
```

### Quick status check:
```bash
./monitor_tests.sh
```

### Check which tests completed:
```bash
ls -lh suara_2d_results/suara_2d_results_P*.md
```

### View master summary:
```bash
cat suara_2d_results/master_summary.md
```

## Result Files

Each successful test generates:
- `suara_2d_results/suara_2d_results_P{N}.md` - Detailed results for P=N processes
- Updated `suara_2d_results/master_summary.md` - Summary of all tests

## What to Expect

### Successful Tests (likely):
- **P=128**: Should complete ✓
- **P=256**: Should complete ✓
- **P=512**: Should complete (but slower)
- **P=1024**: May complete (very slow)

### Potential Failures (resource limits):
- **P=2048**: High risk of failure
- **P=4096**: Very high risk of failure

On an 8-core machine with oversubscription:
- Heavy CPU usage
- Slower response times
- Possible memory pressure

## Key Findings to Look For

When tests complete, check the results for:

1. **Speedup trends**: Does speedup decrease as P increases?
2. **Symmetric vs Asymmetric**: Which performs better?
3. **Algorithm comparison**: Ring+Ring vs Rabenseifner+Rabenseifner
4. **Scaling behavior**: How do times scale with P?

## If You Need to Stop Tests

```bash
# Stop all tests immediately
pkill -f run_large_tests.sh
pkill -f mpirun

# Or just wait - they'll complete eventually
```

## After Tests Complete

1. Review `suara_2d_results/master_summary.md`
2. Check individual result files for detailed performance data
3. Look for patterns in best configurations
4. Compare results across different P values

## Important Notes

⚠️ **System Impact**: Running 4096 processes on an 8-core machine means:
- 512× oversubscription
- Very high context switching overhead
- Expected significant slowdown
- Results may not be representative of real distributed systems

✅ **Valid Ranges**: Results for P=4 through P=512 should be most reliable and meaningful.

## Files in This Directory

- `suara_2d_comprehensive.c` - Main implementation
- `run_large_tests.sh` - Test automation script
- `monitor_tests.sh` - Progress monitoring script
- `large_test_run.log` - Test execution log
- `suara_2d_results/` - Directory with all result files
- `README_2D_SUARA.md` - Original documentation

---
*Updated: $(date)*
