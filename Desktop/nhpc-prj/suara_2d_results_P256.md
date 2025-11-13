# 2D SUARA Allreduce Results - P=256

## Configuration

- **Number of processes:** 256
- **Message size:** 1059840 integers (4.04 MB)
- **Warmup iterations:** 2
- **Timing iterations:** 10

## Valid 2D Grid Factorizations

| No. | R | C | Type |
|-----|---|---|------------|
| 1 | 1 | 256 | Asymmetric |
| 2 | 2 | 128 | Asymmetric |
| 3 | 4 | 64 | Asymmetric |
| 4 | 8 | 32 | Asymmetric |
| 5 | 16 | 16 | Symmetric |

## Baseline Performance

- **Standard MPI_Allreduce:** 0.829612 seconds

## Detailed Performance Analysis

| Grid | R×C | Row Algorithm | Col Algorithm | Time (s) | Speedup | Status |
|------|-----|---------------|---------------|----------|---------|--------|
| 1 | 1x256 | Ring | Ring | 1.772217 | 0.47x | ✓ PASS |
| 1 | 1x256 | Ring | Rabenseifner | 1.869183 | 0.44x | ✓ PASS |
| 1 | 1x256 | Rabenseifner | Ring | 1.440101 | 0.58x | ✓ PASS |
| 1 | 1x256 | Rabenseifner | Rabenseifner | 1.286434 | 0.64x | ✓ PASS |
| 2 | 2x128 | Ring | Ring | 1.431429 | 0.58x | ✓ PASS |
| 2 | 2x128 | Ring | Rabenseifner | 1.502123 | 0.55x | ✓ PASS |
| 2 | 2x128 | Rabenseifner | Ring | 1.264393 | 0.66x | ✓ PASS |
| 2 | 2x128 | Rabenseifner | Rabenseifner | 1.331940 | 0.62x | ✓ PASS |
| 3 | 4x64 | Ring | Ring | 1.393683 | 0.60x | ✓ PASS |
| 3 | 4x64 | Ring | Rabenseifner | 1.400909 | 0.59x | ✓ PASS |
| 3 | 4x64 | Rabenseifner | Ring | 0.940133 | 0.88x | ✓ PASS |
| 3 | 4x64 | Rabenseifner | Rabenseifner | 1.055497 | 0.79x | ✓ PASS |
| 4 | 8x32 | Ring | Ring | 1.533605 | 0.54x | ✓ PASS |
| 4 | 8x32 | Ring | Rabenseifner | 1.327245 | 0.63x | ✓ PASS |
| 4 | 8x32 | Rabenseifner | Ring | 0.858772 | 0.97x | ✓ PASS |
| 4 | 8x32 | Rabenseifner | Rabenseifner | 0.865825 | 0.96x | ✓ PASS |
| 5 | 16x16 | Ring | Ring | 1.362492 | 0.61x | ✓ PASS |
| 5 | 16x16 | Ring | Rabenseifner | 1.345703 | 0.62x | ✓ PASS |
| 5 | 16x16 | Rabenseifner | Ring | 1.174050 | 0.71x | ✓ PASS |
| 5 | 16x16 | Rabenseifner | Rabenseifner | 1.183504 | 0.70x | ✓ PASS |

## Summary and Key Findings

### Best Configuration

- **Grid:** 8 × 32
- **Row Algorithm:** Rabenseifner
- **Column Algorithm:** Ring
- **Time:** 0.858772 seconds
- **Speedup vs Standard:** 0.97x


### Key Insights

1. Symmetric configurations (√P × √P) work well for Ring+Ring
2. Rabenseifner pairs show less dependence on grid shape
3. Mixed algorithm pairs may benefit from asymmetric grids
4. Best configuration balances algorithm and grid geometry

---
*Generated on: Nov 11 2025*
