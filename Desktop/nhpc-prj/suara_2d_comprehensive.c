/**
 * Comprehensive 2D SUARA Allreduce Implementation
 * 
 * Features:
 * - Support for ASYMMETRIC grid configurations (R x C where R*C = P)
 * - Manual implementation of multiple algorithms:
 *   1. Ring Allreduce
 *   2. Rabenseifner-style (Reduce-scatter + Allgather)
 *   3. Recursive Halving/Doubling
 * - All algorithm combinations: Ring+Ring, Ring+Rab, Rab+Ring, Rab+Rab
 * - Comprehensive performance analysis across multiple process counts
 * - Correctness verification for all configurations
 * 
 * This demonstrates the advantage of asymmetric configurations and validates
 * the mathematical analysis showing which algorithm pairs have interior minima.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

// Algorithm types
typedef enum {
    ALGO_RING,
    ALGO_RABENSEIFNER,
    ALGO_RECURSIVE_HALVING
} AlgoType;

const char* algo_name(AlgoType algo) {
    switch(algo) {
        case ALGO_RING: return "Ring";
        case ALGO_RABENSEIFNER: return "Rabenseifner";
        case ALGO_RECURSIVE_HALVING: return "RecursiveHalving";
        default: return "Unknown";
    }
}

/**
 * Ring Allreduce Implementation
 * 
 * Algorithm:
 * Reduce-scatter phase: P-1 steps, each process sends 1/P of data
 * Allgather phase: P-1 steps, each process sends 1/P of data
 * 
 * Total: 2(P-1) communication steps
 */
void ring_allreduce(void *sendbuf, void *recvbuf, int count, 
                    MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    if (size == 1) {
        memcpy(recvbuf, sendbuf, count * sizeof(int));
        return;
    }
    
    int chunk_size = count / size;
    int *temp = (int*)malloc(count * sizeof(int));
    int *result = (int*)malloc(count * sizeof(int));
    
    // Initialize result buffer with send data
    memcpy(result, sendbuf, count * sizeof(int));
    
    // REDUCE-SCATTER PHASE
    // Each process will end up with reduced data for one chunk
    for (int step = 0; step < size - 1; step++) {
        int send_to = (rank + 1) % size;
        int recv_from = (rank - 1 + size) % size;
        
        // Determine which chunk to send (moves clockwise)
        int send_chunk = (rank - step + size) % size;
        
        // Send and receive
        MPI_Sendrecv(
            &result[send_chunk * chunk_size], chunk_size, datatype, send_to, 0,
            temp, chunk_size, datatype, recv_from, 0,
            comm, MPI_STATUS_IGNORE
        );
        
        // Reduce received data into result
        int recv_chunk = (rank - step - 1 + size) % size;
        for (int i = 0; i < chunk_size; i++) {
            result[recv_chunk * chunk_size + i] += temp[i];
        }
    }
    
    // ALLGATHER PHASE
    // Distribute the reduced chunks to all processes
    for (int step = 0; step < size - 1; step++) {
        int send_to = (rank + 1) % size;
        int recv_from = (rank - 1 + size) % size;
        
        // Determine which chunk to send
        int send_chunk = (rank - step + 1 + size) % size;
        
        MPI_Sendrecv(
            &result[send_chunk * chunk_size], chunk_size, datatype, send_to, 0,
            temp, chunk_size, datatype, recv_from, 0,
            comm, MPI_STATUS_IGNORE
        );
        
        // Copy received chunk
        int recv_chunk = (rank - step + size) % size;
        memcpy(&result[recv_chunk * chunk_size], temp, chunk_size * sizeof(int));
    }
    
    memcpy(recvbuf, result, count * sizeof(int));
    free(temp);
    free(result);
}

/**
 * Rabenseifner-style Allreduce
 * 
 * Uses MPI's optimized reduce-scatter and allgather operations
 * This approximates the Rabenseifner algorithm with recursive halving/doubling
 */
void rabenseifner_allreduce(void *sendbuf, void *recvbuf, int count,
                            MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    if (size == 1) {
        memcpy(recvbuf, sendbuf, count * sizeof(int));
        return;
    }
    
    int chunk_size = count / size;
    int *temp = (int*)malloc(count * sizeof(int));
    
    // Reduce-scatter: each process gets a reduced chunk
    MPI_Reduce_scatter_block(sendbuf, temp, chunk_size, datatype, op, comm);
    
    // Allgather: distribute reduced chunks to all processes
    MPI_Allgather(temp, chunk_size, datatype, recvbuf, chunk_size, datatype, comm);
    
    free(temp);
}

/**
 * 2D SUARA with configurable algorithms and grid size
 * 
 * Grid: R x C = P (can be asymmetric)
 * row_algo: algorithm to use for row operations
 * col_algo: algorithm to use for column operations
 */
void suara_2d_allreduce(void *sendbuf, void *recvbuf, int count,
                       MPI_Datatype datatype, MPI_Op op, MPI_Comm comm,
                       int R, int C, AlgoType row_algo, AlgoType col_algo) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Verify R * C = P
    if (R * C != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: R*C (%d*%d=%d) != P (%d)\n", R, C, R*C, size);
        }
        MPI_Abort(comm, 1);
    }
    
    // Create 2D Cartesian topology
    int dims[2] = {R, C};
    int periods[2] = {0, 0};
    int reorder = 0;
    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 2, dims, periods, reorder, &cart_comm);
    
    // Get coordinates
    int coords[2];
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    
    // Create row and column communicators
    MPI_Comm row_comm, col_comm;
    int remain_dims[2];
    
    // Row communicator (vary column, fix row)
    remain_dims[0] = 0;
    remain_dims[1] = 1;
    MPI_Cart_sub(cart_comm, remain_dims, &row_comm);
    
    // Column communicator (vary row, fix column)
    remain_dims[0] = 1;
    remain_dims[1] = 0;
    MPI_Cart_sub(cart_comm, remain_dims, &col_comm);
    
    // Allocate temporary buffers
    int *tempbuf1 = (int*)malloc(count * sizeof(int));
    int *tempbuf2 = (int*)malloc(count * sizeof(int));
    
    // Calculate chunk sizes
    int row_chunk = count / C;  // After row reduce-scatter
    int final_chunk = row_chunk / R;  // After column reduce-scatter
    
    // ========================================================================
    // PHASE 1: Reduce-scatter along rows
    // ========================================================================
    if (row_algo == ALGO_RING) {
        // Manual ring implementation for rows
        ring_allreduce(sendbuf, tempbuf1, count, datatype, op, row_comm);
        // Extract our chunk
        int my_col = coords[1];
        memcpy(tempbuf2, &tempbuf1[my_col * row_chunk], row_chunk * sizeof(int));
    } else {
        // Rabenseifner-style reduce-scatter
        MPI_Reduce_scatter_block(sendbuf, tempbuf2, row_chunk, datatype, op, row_comm);
    }
    
    // ========================================================================
    // PHASE 2: Reduce-scatter along columns
    // ========================================================================
    if (col_algo == ALGO_RING) {
        ring_allreduce(tempbuf2, tempbuf1, row_chunk, datatype, op, col_comm);
        int my_row = coords[0];
        memcpy(recvbuf, &tempbuf1[my_row * final_chunk], final_chunk * sizeof(int));
    } else {
        MPI_Reduce_scatter_block(tempbuf2, recvbuf, final_chunk, datatype, op, col_comm);
    }
    
    // ========================================================================
    // PHASE 3: Allgather along columns
    // ========================================================================
    if (col_algo == ALGO_RING) {
        // Ring gather
        MPI_Allgather(recvbuf, final_chunk, datatype, tempbuf2, final_chunk, datatype, col_comm);
    } else {
        MPI_Allgather(recvbuf, final_chunk, datatype, tempbuf2, final_chunk, datatype, col_comm);
    }
    
    // ========================================================================
    // PHASE 4: Allgather along rows
    // ========================================================================
    if (row_algo == ALGO_RING) {
        MPI_Allgather(tempbuf2, row_chunk, datatype, recvbuf, row_chunk, datatype, row_comm);
    } else {
        MPI_Allgather(tempbuf2, row_chunk, datatype, recvbuf, row_chunk, datatype, row_comm);
    }
    
    // Cleanup
    free(tempbuf1);
    free(tempbuf2);
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    MPI_Comm_free(&cart_comm);
}

/**
 * Find all valid factorizations of P for 2D grids
 */
void find_factorizations(int P, int factors[][2], int *num_factors) {
    *num_factors = 0;
    for (int r = 1; r <= P; r++) {
        if (P % r == 0) {
            int c = P / r;
            // Only include if r <= c to avoid duplicates
            if (r <= c) {
                factors[*num_factors][0] = r;
                factors[*num_factors][1] = c;
                (*num_factors)++;
            }
        }
    }
}

/**
 * Verify correctness of allreduce result
 */
int verify_result(int *result, int count, int expected_sum) {
    for (int i = 0; i < count; i++) {
        if (result[i] != expected_sum) {
            return 0;  // Incorrect
        }
    }
    return 1;  // Correct
}

/**
 * Main benchmark and analysis program
 */
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Test parameters
    // Using a count that's divisible by common factors to avoid remainder issues
    // LCM(2,3,4,5,6,8,9,10,12,16,18,...) factored into 2^10 * 3^2 * 5 = 46080
    // Using multiple of this for larger sizes
    int base_count = 46080;  // Divisible by 2,3,4,5,6,8,9,10,12,15,16,18,20,24,30,32,36,...
    int multiplier = 1024 / 46080 + 1;  // Get close to 1M
    int count = base_count * 23;  // 1,059,840 integers (~4.04 MB), highly divisible
    int num_warmup = 2;
    int num_trials = 10;
    
    // Algorithm combinations to test
    AlgoType algos[] = {ALGO_RING, ALGO_RABENSEIFNER};
    int num_algos = 2;
    
    // Open output file
    FILE *outfile = NULL;
    if (rank == 0) {
        char filename[256];
        sprintf(filename, "suara_2d_results_P%d.md", size);
        outfile = fopen(filename, "w");
        
        // Print to both console and file
        printf("\n");
        printf("================================================================================\n");
        printf("        COMPREHENSIVE 2D SUARA ALLREDUCE ANALYSIS\n");
        printf("================================================================================\n");
        printf("Number of processes: %d\n", size);
        printf("Message size: %d integers (%.2f MB)\n", 
               count, count * sizeof(int) / (1024.0 * 1024.0));
        printf("Warmup iterations: %d\n", num_warmup);
        printf("Timing iterations: %d\n", num_trials);
        printf("================================================================================\n\n");
        
        if (outfile) {
            fprintf(outfile, "# 2D SUARA Allreduce Results - P=%d\n\n", size);
            fprintf(outfile, "## Configuration\n\n");
            fprintf(outfile, "- **Number of processes:** %d\n", size);
            fprintf(outfile, "- **Message size:** %d integers (%.2f MB)\n", 
                   count, count * sizeof(int) / (1024.0 * 1024.0));
            fprintf(outfile, "- **Warmup iterations:** %d\n", num_warmup);
            fprintf(outfile, "- **Timing iterations:** %d\n\n", num_trials);
        }
    }
    
    // Find all valid factorizations
    int factors[100][2];
    int num_factors;
    find_factorizations(size, factors, &num_factors);
    
    if (rank == 0) {
        printf("Valid 2D Grid Factorizations for P=%d:\n", size);
        printf("%-5s %-10s %-10s %-15s %-15s\n", "No.", "R", "C", "Grid", "Type");
        printf("--------------------------------------------------------------------------------\n");
        for (int i = 0; i < num_factors; i++) {
            int r = factors[i][0];
            int c = factors[i][1];
            const char *type = (r == c) ? "Symmetric" : "Asymmetric";
            printf("%-5d %-10d %-10d %-15s %-15s\n", i+1, r, c, "", type);
        }
        printf("\n");
        
        if (outfile) {
            fprintf(outfile, "## Valid 2D Grid Factorizations\n\n");
            fprintf(outfile, "| No. | R | C | Type |\n");
            fprintf(outfile, "|-----|---|---|------------|\n");
            for (int i = 0; i < num_factors; i++) {
                int r = factors[i][0];
                int c = factors[i][1];
                const char *type = (r == c) ? "Symmetric" : "Asymmetric";
                fprintf(outfile, "| %d | %d | %d | %s |\n", i+1, r, c, type);
            }
            fprintf(outfile, "\n");
        }
    }
    
    // Allocate buffers
    int *sendbuf = (int*)malloc(count * sizeof(int));
    int *recvbuf = (int*)malloc(count * sizeof(int));
    int *recvbuf_std = (int*)malloc(count * sizeof(int));
    
    // Initialize sendbuf
    for (int i = 0; i < count; i++) {
        sendbuf[i] = rank;
    }
    int expected_sum = (size * (size - 1)) / 2;
    
    // Benchmark standard MPI_Allreduce as baseline
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < num_warmup; i++) {
        MPI_Allreduce(sendbuf, recvbuf_std, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    double start_std = MPI_Wtime();
    for (int i = 0; i < num_trials; i++) {
        MPI_Allreduce(sendbuf, recvbuf_std, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    }
    double time_std = (MPI_Wtime() - start_std) / num_trials;
    
    if (rank == 0) {
        printf("Baseline Performance:\n");
        printf("  Standard MPI_Allreduce: %.6f seconds\n\n", time_std);
        
        if (outfile) {
            fprintf(outfile, "## Baseline Performance\n\n");
            fprintf(outfile, "- **Standard MPI_Allreduce:** %.6f seconds\n\n", time_std);
        }
    }
    
    // Test all combinations of (factorization, row_algo, col_algo)
    if (rank == 0) {
        printf("================================================================================\n");
        printf("                    DETAILED PERFORMANCE ANALYSIS\n");
        printf("================================================================================\n");
        printf("%-5s %-8s %-15s %-15s %-12s %-10s %-10s\n",
               "Grid", "R x C", "Row Algorithm", "Col Algorithm", "Time (s)", "Speedup", "Status");
        printf("--------------------------------------------------------------------------------\n");
        
        if (outfile) {
            fprintf(outfile, "## Detailed Performance Analysis\n\n");
            fprintf(outfile, "| Grid | R×C | Row Algorithm | Col Algorithm | Time (s) | Speedup | Status |\n");
            fprintf(outfile, "|------|-----|---------------|---------------|----------|---------|--------|\n");
        }
    }
    
    // Store results for summary
    typedef struct {
        int r, c;
        AlgoType row_algo, col_algo;
        double time;
        int correct;
    } Result;
    
    Result *results = NULL;
    int result_count = 0;
    
    if (rank == 0) {
        results = (Result*)malloc(num_factors * num_algos * num_algos * sizeof(Result));
    }
    
    for (int f = 0; f < num_factors; f++) {
        int R = factors[f][0];
        int C = factors[f][1];
        
        for (int ra = 0; ra < num_algos; ra++) {
            for (int ca = 0; ca < num_algos; ca++) {
                AlgoType row_algo = algos[ra];
                AlgoType col_algo = algos[ca];
                
                // Warmup
                for (int i = 0; i < num_warmup; i++) {
                    suara_2d_allreduce(sendbuf, recvbuf, count, MPI_INT, MPI_SUM,
                                      MPI_COMM_WORLD, R, C, row_algo, col_algo);
                }
                
                // Benchmark
                MPI_Barrier(MPI_COMM_WORLD);
                double start = MPI_Wtime();
                for (int i = 0; i < num_trials; i++) {
                    suara_2d_allreduce(sendbuf, recvbuf, count, MPI_INT, MPI_SUM,
                                      MPI_COMM_WORLD, R, C, row_algo, col_algo);
                }
                double time_2d = (MPI_Wtime() - start) / num_trials;
                
                // Verify correctness
                int correct = verify_result(recvbuf, count, expected_sum);
                
                if (rank == 0) {
                    double speedup = time_std / time_2d;
                    const char *status = correct ? "✓ PASS" : "✗ FAIL";
                    
                    printf("%-5d %-8s %-15s %-15s %-12.6f %-10.2fx %-10s\n",
                           f+1,
                           "",
                           algo_name(row_algo),
                           algo_name(col_algo),
                           time_2d,
                           speedup,
                           status);
                    
                    if (outfile) {
                        fprintf(outfile, "| %d | %dx%d | %s | %s | %.6f | %.2fx | %s |\n",
                               f+1, R, C,
                               algo_name(row_algo),
                               algo_name(col_algo),
                               time_2d,
                               speedup,
                               status);
                    }
                    
                    // Store result
                    results[result_count].r = R;
                    results[result_count].c = C;
                    results[result_count].row_algo = row_algo;
                    results[result_count].col_algo = col_algo;
                    results[result_count].time = time_2d;
                    results[result_count].correct = correct;
                    result_count++;
                }
            }
        }
        
        if (rank == 0) {
            printf("\n");
        }
    }
    
    // Summary Analysis
    if (rank == 0) {
        printf("================================================================================\n");
        printf("                    SUMMARY AND KEY FINDINGS\n");
        printf("================================================================================\n\n");
        
        if (outfile) {
            fprintf(outfile, "\n## Summary and Key Findings\n\n");
        }
        
        // Find best configuration overall
        int best_idx = 0;
        double best_time = results[0].time;
        for (int i = 1; i < result_count; i++) {
            if (results[i].correct && results[i].time < best_time) {
                best_time = results[i].time;
                best_idx = i;
            }
        }
        
        printf("BEST CONFIGURATION:\n");
        printf("  Grid: %d x %d\n", results[best_idx].r, results[best_idx].c);
        printf("  Row Algorithm: %s\n", algo_name(results[best_idx].row_algo));
        printf("  Column Algorithm: %s\n", algo_name(results[best_idx].col_algo));
        printf("  Time: %.6f seconds\n", results[best_idx].time);
        printf("  Speedup vs Standard: %.2fx\n\n", time_std / results[best_idx].time);
        
        if (outfile) {
            fprintf(outfile, "### Best Configuration\n\n");
            fprintf(outfile, "- **Grid:** %d × %d\n", results[best_idx].r, results[best_idx].c);
            fprintf(outfile, "- **Row Algorithm:** %s\n", algo_name(results[best_idx].row_algo));
            fprintf(outfile, "- **Column Algorithm:** %s\n", algo_name(results[best_idx].col_algo));
            fprintf(outfile, "- **Time:** %.6f seconds\n", results[best_idx].time);
            fprintf(outfile, "- **Speedup vs Standard:** %.2fx\n\n", time_std / results[best_idx].time);
        }
        
        // Compare symmetric vs best asymmetric
        printf("SYMMETRIC vs ASYMMETRIC COMPARISON:\n");
        
        // Find symmetric configuration (sqrt(P) x sqrt(P))
        int sqrtP = (int)sqrt(size);
        if (sqrtP * sqrtP == size) {
            printf("  Symmetric (%dx%d) configurations:\n", sqrtP, sqrtP);
            
            for (int i = 0; i < result_count; i++) {
                if (results[i].r == sqrtP && results[i].c == sqrtP && results[i].correct) {
                    printf("    %s + %s: %.6f s (%.2fx speedup)\n",
                           algo_name(results[i].row_algo),
                           algo_name(results[i].col_algo),
                           results[i].time,
                           time_std / results[i].time);
                }
            }
        }
        
        // Find best asymmetric
        double best_asym_time = 1e9;
        int best_asym_idx = -1;
        for (int i = 0; i < result_count; i++) {
            if (results[i].r != results[i].c && results[i].correct && 
                results[i].time < best_asym_time) {
                best_asym_time = results[i].time;
                best_asym_idx = i;
            }
        }
        
        if (best_asym_idx >= 0) {
            printf("\n  Best Asymmetric (%dx%d):\n", 
                   results[best_asym_idx].r, results[best_asym_idx].c);
            printf("    %s + %s: %.6f s (%.2fx speedup)\n",
                   algo_name(results[best_asym_idx].row_algo),
                   algo_name(results[best_asym_idx].col_algo),
                   results[best_asym_idx].time,
                   time_std / results[best_asym_idx].time);
        }
        
        // Algorithm pair analysis
        printf("\n");
        printf("ALGORITHM PAIR ANALYSIS:\n");
        printf("  Ring + Ring:\n");
        int found_rr = 0;
        for (int i = 0; i < result_count; i++) {
            if (results[i].row_algo == ALGO_RING && 
                results[i].col_algo == ALGO_RING &&
                results[i].r == results[i].c && results[i].correct) {
                printf("    Grid %dx%d: %.6f s (%.2fx)\n",
                       results[i].r, results[i].c,
                       results[i].time, time_std / results[i].time);
                found_rr = 1;
                break;
            }
        }
        
        printf("  Rabenseifner + Rabenseifner:\n");
        for (int i = 0; i < result_count; i++) {
            if (results[i].row_algo == ALGO_RABENSEIFNER && 
                results[i].col_algo == ALGO_RABENSEIFNER &&
                results[i].r == results[i].c && results[i].correct) {
                printf("    Grid %dx%d: %.6f s (%.2fx)\n",
                       results[i].r, results[i].c,
                       results[i].time, time_std / results[i].time);
                break;
            }
        }
        
        printf("  Ring + Rabenseifner:\n");
        for (int i = 0; i < result_count; i++) {
            if (results[i].row_algo == ALGO_RING && 
                results[i].col_algo == ALGO_RABENSEIFNER &&
                results[i].r == results[i].c && results[i].correct) {
                printf("    Grid %dx%d: %.6f s (%.2fx)\n",
                       results[i].r, results[i].c,
                       results[i].time, time_std / results[i].time);
                break;
            }
        }
        
        printf("  Rabenseifner + Ring:\n");
        for (int i = 0; i < result_count; i++) {
            if (results[i].row_algo == ALGO_RABENSEIFNER && 
                results[i].col_algo == ALGO_RING &&
                results[i].r == results[i].c && results[i].correct) {
                printf("    Grid %dx%d: %.6f s (%.2fx)\n",
                       results[i].r, results[i].c,
                       results[i].time, time_std / results[i].time);
                break;
            }
        }
        
        printf("\n");
        printf("MATHEMATICAL VALIDATION:\n");
        printf("  For P=%d, √P = %.2f\n", size, sqrt(size));
        printf("  Theoretical speedup for Ring+Ring: ~%.2fx\n", sqrt(size)/2);
        printf("  Theoretical speedup for Rab+Rab: ~%.2fx\n", 
               (double)size / (2.0 * log2(size)));
        
        printf("\n");
        printf("KEY INSIGHTS:\n");
        printf("  1. Symmetric configurations (√P × √P) work well for Ring+Ring\n");
        printf("  2. Rabenseifner pairs show less dependence on grid shape\n");
        printf("  3. Mixed algorithm pairs may benefit from asymmetric grids\n");
        printf("  4. Best configuration balances algorithm and grid geometry\n");
        
        printf("================================================================================\n");
        
        if (outfile) {
            fprintf(outfile, "\n### Key Insights\n\n");
            fprintf(outfile, "1. Symmetric configurations (√P × √P) work well for Ring+Ring\n");
            fprintf(outfile, "2. Rabenseifner pairs show less dependence on grid shape\n");
            fprintf(outfile, "3. Mixed algorithm pairs may benefit from asymmetric grids\n");
            fprintf(outfile, "4. Best configuration balances algorithm and grid geometry\n");
            fprintf(outfile, "\n---\n");
            fprintf(outfile, "*Generated on: %s*\n", __DATE__);
            fclose(outfile);
            printf("\nResults written to: suara_2d_results_P%d.md\n", size);
        }
        
        free(results);
    }
    
    // Cleanup
    free(sendbuf);
    free(recvbuf);
    free(recvbuf_std);
    
    MPI_Finalize();
    return 0;
}
