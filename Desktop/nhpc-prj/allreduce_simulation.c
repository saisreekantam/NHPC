#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Network parameters
typedef struct {
    double alpha;      // Latency (microseconds)
    double beta;       // Inverse bandwidth (microseconds per byte)
} NetworkParams;

// Simulation result structure
typedef struct {
    char algorithm[50];
    int dimensions;
    int num_processes;
    double time_us;     // Time in microseconds
    int num_steps;
    double avg_msg_size;
} SimResult;

// Function prototypes
double simulate_ring_1d(int P, size_t M, NetworkParams net);
double simulate_rabenseifner_1d(int P, size_t M, NetworkParams net);
double simulate_2d_ring(int P, size_t M, NetworkParams net);
double simulate_2d_rabenseifner(int P, size_t M, NetworkParams net);
double simulate_3d_ring(int P, size_t M, NetworkParams net);
double simulate_3d_rabenseifner(int P, size_t M, NetworkParams net);
double simulate_kd_ring(int P, size_t M, int k, NetworkParams net);
double simulate_kd_rabenseifner(int P, size_t M, int k, NetworkParams net);
int is_perfect_power(int n, int k);
int int_pow(int base, int exp);
void print_results_table(SimResult *results, int count);

/**
 * Standard 1D Ring Allreduce
 * Algorithm:
 * - Reduce-scatter phase: (P-1) steps, each sends M/P data
 * - Allgather phase: (P-1) steps, each sends M/P data
 * 
 * Time = 2(P-1) * (Î± + (M/P)/Î²)
 */
double simulate_ring_1d(int P, size_t M, NetworkParams net) {
    if (P <= 1) return 0.0;
    
    int steps = 2 * (P - 1);  // reduce-scatter + allgather
    double msg_size = (double)M / P;
    double time = steps * (net.alpha + msg_size * net.beta);
    
    return time;
}

/**
 * Standard 1D Rabenseifner Allreduce
 * Algorithm:
 * - Reduce-scatter: logâ‚‚(P) steps with recursive halving
 * - Allgather: logâ‚‚(P) steps with recursive doubling
 * 
 * For reduce-scatter:
 *   Step i: message size = M / (2^(i+1))
 *   Total: sum from i=0 to logâ‚‚(P)-1 of M / 2^(i+1) = M * (1 - 1/P)
 * 
 * Time = 2*logâ‚‚(P)*Î± + 2*M*(1 - 1/P)/Î²
 */
double simulate_rabenseifner_1d(int P, size_t M, NetworkParams net) {
    if (P <= 1) return 0.0;
    if ((P & (P - 1)) != 0) {
        // Not a power of 2, use approximation
        fprintf(stderr, "Warning: P=%d not power of 2 for Rabenseifner\n", P);
    }
    
    int log_p = (int)(log2(P) + 0.5);
    double latency_term = 2 * log_p * net.alpha;
    double bandwidth_term = 2 * M * (1.0 - 1.0/P) * net.beta;
    
    return latency_term + bandwidth_term;
}

/**
 * 2D SUARA with Ring in each dimension
 * Grid: âˆšP Ã— âˆšP
 * 
 * Algorithm:
 * 1. Reduce-scatter along rows: (âˆšP - 1) steps per row
 * 2. Reduce-scatter along columns: (âˆšP - 1) steps per column  
 * 3. Allgather along columns: (âˆšP - 1) steps per column
 * 4. Allgather along rows: (âˆšP - 1) steps per row
 * 
 * Message size per step: M/P (each process handles M/P data)
 * 
 * Time = 4(âˆšP - 1) * (Î± + (M/P)/Î²)
 */
double simulate_2d_ring(int P, size_t M, NetworkParams net) {
    if (P <= 1) return 0.0;
    
    int sqrtP = (int)(sqrt(P) + 0.5);
    if (sqrtP * sqrtP != P) {
        fprintf(stderr, "Warning: P=%d is not perfect square for 2D\n", P);
        return -1.0;
    }
    
    int steps_per_phase = sqrtP - 1;
    int total_steps = 4 * steps_per_phase;
    double msg_size = (double)M / P;
    double time = total_steps * (net.alpha + msg_size * net.beta);
    
    return time;
}

/**
 * 2D SUARA with Rabenseifner in each dimension
 * Grid: âˆšP Ã— âˆšP
 * 
 * Algorithm:
 * Each dimension uses Rabenseifner (reduce-scatter + allgather)
 * 
 * For dimension with âˆšP processes:
 * - Reduce-scatter: logâ‚‚(âˆšP) steps
 * - Allgather: logâ‚‚(âˆšP) steps
 * 
 * Total: 4 phases Ã— logâ‚‚(âˆšP) steps
 *      = 4 Ã— (logâ‚‚(P)/2) = 2*logâ‚‚(P) steps
 * 
 * Bandwidth calculation (per process):
 * Each process handles M/P data through all phases
 * Total data movement â‰ˆ 2*M*(1 - 1/âˆšP) per dimension Ã— 2 dimensions
 * 
 * Time = 2*logâ‚‚(P)*Î± + 4*M*(1 - 1/âˆšP)*(1/P)*Î²
 */
double simulate_2d_rabenseifner(int P, size_t M, NetworkParams net) {
    if (P <= 1) return 0.0;
    
    int sqrtP = (int)(sqrt(P) + 0.5);
    if (sqrtP * sqrtP != P) {
        fprintf(stderr, "Warning: P=%d is not perfect square for 2D\n", P);
        return -1.0;
    }
    
    int log_p = (int)(log2(P) + 0.5);
    double latency_term = 2 * log_p * net.alpha;
    
    // Bandwidth term: 4 phases, each moves M/âˆšP data with (1-1/âˆšP) efficiency
    double bandwidth_term = 4 * M * (1.0 - 1.0/sqrtP) / sqrtP * net.beta;
    
    return latency_term + bandwidth_term;
}

/**
 * 3D SUARA with Ring in each dimension
 * Grid: âˆ›P Ã— âˆ›P Ã— âˆ›P
 * 
 * Algorithm:
 * 6 phases (reduce-scatter + allgather for each of 3 dimensions)
 * 
 * Time = 6(âˆ›P - 1) * (Î± + (M/P)/Î²)
 */
double simulate_3d_ring(int P, size_t M, NetworkParams net) {
    if (P <= 1) return 0.0;
    
    int cbrtP = (int)(pow(P, 1.0/3.0) + 0.5);
    if (cbrtP * cbrtP * cbrtP != P) {
        fprintf(stderr, "Warning: P=%d is not perfect cube for 3D\n", P);
        return -1.0;
    }
    
    int steps_per_phase = cbrtP - 1;
    int total_steps = 6 * steps_per_phase;
    double msg_size = (double)M / P;
    double time = total_steps * (net.alpha + msg_size * net.beta);
    
    return time;
}

/**
 * 3D SUARA with Rabenseifner in each dimension
 * Grid: âˆ›P Ã— âˆ›P Ã— âˆ›P
 * 
 * Total: 6 phases Ã— logâ‚‚(âˆ›P) steps
 *      = 6 Ã— (logâ‚‚(P)/3) = 2*logâ‚‚(P) steps
 * 
 * Time = 2*logâ‚‚(P)*Î± + 6*M*(1 - 1/âˆ›P)*(1/P^(2/3))*Î²
 */
double simulate_3d_rabenseifner(int P, size_t M, NetworkParams net) {
    if (P <= 1) return 0.0;
    
    int cbrtP = (int)(pow(P, 1.0/3.0) + 0.5);
    if (cbrtP * cbrtP * cbrtP != P) {
        fprintf(stderr, "Warning: P=%d is not perfect cube for 3D\n", P);
        return -1.0;
    }
    
    int log_p = (int)(log2(P) + 0.5);
    double latency_term = 2 * log_p * net.alpha;
    
    // 6 phases, each along âˆ›P processes
    double bandwidth_term = 6 * M * (1.0 - 1.0/cbrtP) / (cbrtP * cbrtP) * net.beta;
    
    return latency_term + bandwidth_term;
}

/**
 * k-D SUARA with Ring in each dimension
 * Grid: P^(1/k) Ã— P^(1/k) Ã— ... (k times)
 * 
 * Algorithm:
 * 2k phases (reduce-scatter + allgather for each of k dimensions)
 * 
 * Time = 2k(P^(1/k) - 1) * (Î± + (M/P)/Î²)
 */
double simulate_kd_ring(int P, size_t M, int k, NetworkParams net) {
    if (P <= 1 || k <= 0) return 0.0;
    
    int p_1_k = (int)(pow(P, 1.0/k) + 0.5);
    
    // Verify it's a perfect k-th power
    if (!is_perfect_power(P, k)) {
        fprintf(stderr, "Warning: P=%d is not perfect %d-th power\n", P, k);
        return -1.0;
    }
    
    int steps_per_phase = p_1_k - 1;
    int total_steps = 2 * k * steps_per_phase;
    double msg_size = (double)M / P;
    double time = total_steps * (net.alpha + msg_size * net.beta);
    
    return time;
}

/**
 * k-D SUARA with Rabenseifner in each dimension
 * Grid: P^(1/k) Ã— P^(1/k) Ã— ... (k times)
 * 
 * Total: 2k phases Ã— logâ‚‚(P^(1/k)) steps
 *      = 2k Ã— (logâ‚‚(P)/k) = 2*logâ‚‚(P) steps
 * 
 * Time = 2*logâ‚‚(P)*Î± + 2k*M*(1 - 1/P^(1/k))*(1/P^((k-1)/k))*Î²
 */
double simulate_kd_rabenseifner(int P, size_t M, int k, NetworkParams net) {
    if (P <= 1 || k <= 0) return 0.0;
    
    int p_1_k = (int)(pow(P, 1.0/k) + 0.5);
    
    if (!is_perfect_power(P, k)) {
        fprintf(stderr, "Warning: P=%d is not perfect %d-th power\n", P, k);
        return -1.0;
    }
    
    int log_p = (int)(log2(P) + 0.5);
    double latency_term = 2 * log_p * net.alpha;
    
    // 2k phases, each along P^(1/k) processes
    double p_inv_k = pow(P, 1.0/k);
    double p_k_1_k = pow(P, (k-1.0)/k);
    double bandwidth_term = 2 * k * M * (1.0 - 1.0/p_inv_k) / p_k_1_k * net.beta;
    
    return latency_term + bandwidth_term;
}

/**
 * Check if n is a perfect k-th power
 */
int is_perfect_power(int n, int k) {
    if (k <= 0) return 0;
    if (k == 1) return 1;
    
    int root = (int)(pow(n, 1.0/k) + 0.5);
    int result = int_pow(root, k);
    
    return result == n;
}

/**
 * Integer power function
 */
int int_pow(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

/**
 * Print results in a formatted table
 */
void print_results_table(SimResult *results, int count) {
    printf("\n");
    printf("================================================================================\n");
    printf("                      ALLREDUCE SIMULATION RESULTS\n");
    printf("================================================================================\n");
    printf("%-25s %5s %8s %12s\n", "Algorithm", "Dim", "Procs", "Time (Î¼s)");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-25s %5dD %8d %12.2f\n", 
               results[i].algorithm,
               results[i].dimensions,
               results[i].num_processes,
               results[i].time_us);
    }
    
    printf("================================================================================\n");
}

/**
 * Print speedup comparison
 */
void print_speedup_analysis(SimResult *results, int count, int P) {
    printf("\n");
    printf("================================================================================\n");
    printf("                    SPEEDUP ANALYSIS (vs 1D Ring)\n");
    printf("================================================================================\n");
    
    // Find 1D Ring baseline
    double baseline = -1.0;
    for (int i = 0; i < count; i++) {
        if (results[i].dimensions == 1 && 
            strcmp(results[i].algorithm, "Ring Allreduce") == 0 &&
            results[i].num_processes == P) {
            baseline = results[i].time_us;
            break;
        }
    }
    
    if (baseline < 0) {
        printf("Baseline (1D Ring) not found\n");
        return;
    }
    
    printf("Baseline: 1D Ring Allreduce = %.2f Î¼s\n\n", baseline);
    printf("%-25s %5s %12s %12s\n", "Algorithm", "Dim", "Time (Î¼s)", "Speedup");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        if (results[i].num_processes == P) {
            double speedup = baseline / results[i].time_us;
            printf("%-25s %5dD %12.2f %12.2fx\n", 
                   results[i].algorithm,
                   results[i].dimensions,
                   results[i].time_us,
                   speedup);
        }
    }
    
    printf("================================================================================\n");
}

/**
 * Main simulation function
 */
int main(int argc, char **argv) {
    // Network parameters (typical values for InfiniBand)
    NetworkParams net = {
        .alpha = 2.0,      // 2 Î¼s latency
        .beta = 0.001      // 1 GB/s bandwidth -> 0.001 Î¼s/byte
    };
    
    // Message size
    size_t M = 1024 * 1024 * 1024;  // 1 GB
    
    // Test different process counts
    int process_counts[] = {64, 256, 1024, 4096};
    int num_test_cases = sizeof(process_counts) / sizeof(process_counts[0]);
    
    printf("\n");
    printf("================================================================================\n");
    printf("               MULTI-DIMENSIONAL ALLREDUCE SIMULATION\n");
    printf("================================================================================\n");
    printf("Network Parameters:\n");
    printf("  Latency (Î±):     %.2f Î¼s\n", net.alpha);
    printf("  Bandwidth:       %.0f GB/s (Î² = %.4f Î¼s/byte)\n", 1.0/net.beta/1000, net.beta);
    printf("  Message Size:    %zu MB\n", M / (1024*1024));
    printf("================================================================================\n");
    
    // Run simulations for each process count
    for (int t = 0; t < num_test_cases; t++) {
        int P = process_counts[t];
        
        printf("\n\n");
        printf("################################################################################\n");
        printf("                    SIMULATION FOR P = %d PROCESSES\n", P);
        printf("################################################################################\n");
        
        SimResult results[20];
        int result_count = 0;
        
        // 1D algorithms
        results[result_count++] = (SimResult){
            .algorithm = "Ring Allreduce",
            .dimensions = 1,
            .num_processes = P,
            .time_us = simulate_ring_1d(P, M, net)
        };
        
        results[result_count++] = (SimResult){
            .algorithm = "Rabenseifner Allreduce",
            .dimensions = 1,
            .num_processes = P,
            .time_us = simulate_rabenseifner_1d(P, M, net)
        };
        
        // 2D algorithms
        int sqrtP = (int)(sqrt(P) + 0.5);
        if (sqrtP * sqrtP == P) {
            results[result_count++] = (SimResult){
                .algorithm = "2D Ring (SUARA)",
                .dimensions = 2,
                .num_processes = P,
                .time_us = simulate_2d_ring(P, M, net)
            };
            
            results[result_count++] = (SimResult){
                .algorithm = "2D Rabenseifner (SUARA)",
                .dimensions = 2,
                .num_processes = P,
                .time_us = simulate_2d_rabenseifner(P, M, net)
            };
        }
        
        // 3D algorithms
        int cbrtP = (int)(pow(P, 1.0/3.0) + 0.5);
        if (cbrtP * cbrtP * cbrtP == P) {
            results[result_count++] = (SimResult){
                .algorithm = "3D Ring (SUARA)",
                .dimensions = 3,
                .num_processes = P,
                .time_us = simulate_3d_ring(P, M, net)
            };
            
            results[result_count++] = (SimResult){
                .algorithm = "3D Rabenseifner (SUARA)",
                .dimensions = 3,
                .num_processes = P,
                .time_us = simulate_3d_rabenseifner(P, M, net)
            };
        }
        
        // 4D, 5D, 6D algorithms (if applicable)
        for (int k = 4; k <= 6; k++) {
            if (is_perfect_power(P, k)) {
                char algo_name[50];
                
                snprintf(algo_name, sizeof(algo_name), "%dD Ring (SUARA)", k);
                results[result_count++] = (SimResult){
                    .dimensions = k,
                    .num_processes = P,
                    .time_us = simulate_kd_ring(P, M, k, net)
                };
                strcpy(results[result_count-1].algorithm, algo_name);
                
                snprintf(algo_name, sizeof(algo_name), "%dD Rabenseifner (SUARA)", k);
                results[result_count++] = (SimResult){
                    .dimensions = k,
                    .num_processes = P,
                    .time_us = simulate_kd_rabenseifner(P, M, k, net)
                };
                strcpy(results[result_count-1].algorithm, algo_name);
            }
        }
        
        // Print results
        print_results_table(results, result_count);
        print_speedup_analysis(results, result_count, P);
        
        // Theoretical analysis
        printf("\n");
        printf("================================================================================\n");
        printf("                      THEORETICAL COMPLEXITY ANALYSIS\n");
        printf("================================================================================\n");
        printf("For P = %d processes:\n\n", P);
        
        printf("1D algorithms:\n");
        printf("  Ring:         O(P) = O(%d) communication steps\n", P);
        printf("  Rabenseifner: O(log P) = O(%.0f) communication steps\n\n", log2(P));
        
        if (sqrtP * sqrtP == P) {
            printf("2D decomposition (%d Ã— %d grid):\n", sqrtP, sqrtP);
            printf("  Ring:         O(âˆšP) = O(%d) communication steps per dimension\n", sqrtP);
            printf("  Rabenseifner: O(log âˆšP) = O(%.0f) steps per dimension\n", log2(sqrtP));
            printf("  Speedup factor: âˆšP / P = %.2fx (latency-dominated)\n\n", sqrt(P));
        }
        
        if (cbrtP * cbrtP * cbrtP == P) {
            printf("3D decomposition (%d Ã— %d Ã— %d grid):\n", cbrtP, cbrtP, cbrtP);
            printf("  Ring:         O(âˆ›P) = O(%d) communication steps per dimension\n", cbrtP);
            printf("  Rabenseifner: O(log âˆ›P) = O(%.0f) steps per dimension\n", log2(cbrtP));
            printf("  Speedup factor: âˆ›P / P = %.2fx (latency-dominated)\n\n", pow(P, 1.0/3.0));
        }
        
        printf("General k-dimensional decomposition:\n");
        printf("  Process arrangement: P^(1/k) processes per dimension\n");
        printf("  Latency term: O(log P) - same for all k\n");
        printf("  Bandwidth term: O(M/P^((k-1)/k)) - improves with k\n");
        printf("  Trade-off: More dimensions = better surface-to-volume ratio\n");
        printf("             but increased coordination overhead\n");
        printf("================================================================================\n");
    }
    
    // Dimensional comparison for a specific P
    printf("\n\n");
    printf("################################################################################\n");
    printf("            DIMENSIONAL SCALING ANALYSIS (P = 4096)\n");
    printf("################################################################################\n");
    
    int P_analysis = 4096;
    
    printf("\nComparing different dimensional decompositions:\n");
    printf("(All values for P = %d, M = %zu MB)\n\n", P_analysis, M/(1024*1024));
    
    // Calculate what dimensions are possible
    printf("Feasible decompositions:\n");
    printf("%-5s %-20s %-15s %-15s\n", "Dim", "Grid Shape", "Procs/Dim", "Is Valid");
    printf("--------------------------------------------------------------------------------\n");
    for (int k = 1; k <= 12; k++) {
        int p_1_k = (int)(pow(P_analysis, 1.0/k) + 0.5);
        int is_valid = is_perfect_power(P_analysis, k);
        
        if (is_valid) {
            char grid_shape[50];
            snprintf(grid_shape, sizeof(grid_shape), "%d^%d", p_1_k, k);
            printf("%-5d %-20s %-15d %-15s\n", k, grid_shape, p_1_k, "Yes");
        }
    }
    
    printf("\n");
    printf("Note: P = 4096 = 2^12 = 64^2 = 16^3 = 8^4 = 4^6\n");
    printf("      Only dimensions that evenly decompose P are feasible.\n");
    printf("================================================================================\n");
    
    return 0;
}