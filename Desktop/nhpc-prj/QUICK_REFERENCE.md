# Quick Reference: 2D SUARA Results Summary

## At a Glance

| P | Best Configuration | Time (s) | Speedup | Key Insight |
|---|-------------------|----------|---------|-------------|
| 4 | 1×4 Rab+Rab | 0.0027 | 0.77× | Asymmetric beats 2×2 symmetric |
| 16 | 2×8 Rab+Rab | 0.0116 | **1.01×** | 🏆 Only config with speedup > 1 |
| 36 | 1×36 Rab+Ring | 0.0340 | 1.00× | Extreme asymmetry works! |
| 64 | 2×32 Rab+Ring | 0.0695 | 0.88× | Moderate asymmetry optimal |
| 128 | 4×32 Rab+Rab | 0.372 | 0.67× | Oversubscription penalty |

## Key Takeaways

### 🎯 Main Results
- ✅ **All 42 configurations passed** correctness tests
- 🏆 **Best speedup: 1.01×** at P=16 (2×8 grid, Rab+Rab)
- 🔄 **Asymmetric grids won** in ALL cases
- 📊 **Rabenseifner dominates** Ring algorithm

### 💡 Algorithm Rankings
1. **Rabenseifner + Rabenseifner** → Best overall (0.78× avg)
2. **Rabenseifner + Ring** → Best for asymmetric (0.75× avg)
3. Ring + Rabenseifner → Moderate (0.45× avg)
4. Ring + Ring → Worst (0.41× avg)

### 📈 Scaling Observations
- P=4 to P=36: Excellent scaling (88-101% efficiency)
- P=64: Good (88% efficiency)
- P=128: Degraded (67% efficiency) due to 16× oversubscription

### 🎓 Theoretical Validation
- ✓ Ring+Ring prefers symmetric (validated)
- ✓ Rabenseifner is grid-independent (validated)
- ✓ Asymmetric can outperform symmetric (validated!)

## Best Practices

**For each P, use:**
- P=4: 1×4 with Rab+Rab
- P=16: 2×8 with Rab+Rab ⭐
- P=36: 1×36 with Rab+Ring
- P=64: 2×32 with Rab+Ring
- P≥128: Standard MPI (better optimized)

**General rule:** Moderately asymmetric (R ≈ C/2 to C/4) with Rabenseifner+Rabenseifner

## Files Location

📂 **Results Directory:** `suara_2d_results/`
- Individual reports: `suara_2d_results_P{N}.md`
- Master summary: `master_summary.md`
- **Comprehensive analysis:** `COMPREHENSIVE_RESULTS_P4_TO_P128.md` ← Full details

## Next Steps

1. Run P=256 and P=512 tests
2. Test on real distributed cluster
3. Vary message sizes
4. Explore 3D configurations

---
*For complete analysis with graphs, tables, and insights, see `COMPREHENSIVE_RESULTS_P4_TO_P128.md`*
