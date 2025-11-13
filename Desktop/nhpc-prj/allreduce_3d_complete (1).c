/**
 * 3D SUARA Allreduce Implementation
 * 
 * Process arrangement: ∛P × ∛P × ∛P grid
 * Algorithm: 6 phases
 *   1. Reduce-scatter along X dimension
 *   2. Reduce-scatter along Y dimension
 *   3. Reduce-scatter along Z dimension
 *   4. Allgather along Z dimension
 *   5. Allgather along Y dimension
 *   6. Allgather along X dimension
 * 
 * Complexity:
 *   Steps: 6(∛P - 1) for Ring, 2×log₂(P) for Rabenseifner
 *   Time: O(∛P × α + M/∛P² × β)
 *   Speedup: ∛P over 1D, ∛P/√P over 2D
 * 
 * Why 3D is "Hard":
 *   - Need 3D Cartesian topology (more complex than 2D)
 *   - Manage 3 separate subcommunicators (X, Y, Z planes)
 *   - 6 communication phases vs 4 in 2D
 *   - More buffer management and coordination
 *   - Harder to debug and visualize
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

void allreduce_3d_suara(void *sendbuf, void *recvbuf, int count,
                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Verify P is a perfect cube
    int cbrtP = (int)(pow(size, 1.0/3.0) + 0.5);
    if (cbrtP * cbrtP * cbrtP != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: P=%d must be perfect cube for 3D SUARA\n", size);
            fprintf(stderr, "Valid values: 8, 27, 64, 125, 216, 343, 512, 729, 1000, ...\n");
        }
        MPI_Abort(comm, 1);
    }
    
    if (rank == 0) {
        printf("3D SUARA: Arranging %d processes in %dx%dx%d grid\n", 
               size, cbrtP, cbrtP, cbrtP);
    }
    
    // Create 3D Cartesian topology
    // dims[0] = X, dims[1] = Y, dims[2] = Z
    int dims[3] = {cbrtP, cbrtP, cbrtP};
    int periods[3] = {0, 0, 0};  // Non-periodic
    int reorder = 0;
    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 3, dims, periods, reorder, &cart_comm);
    
    // Get my coordinates in the 3D grid
    int coords[3];
    MPI_Cart_coords(cart_comm, rank, 3, coords);
    int my_x = coords[0];
    int my_y = coords[1];
    int my_z = coords[2];
    
    if (rank == 0) {
        printf("Example: Rank 0 is at grid position (%d, %d, %d)\n", 
               my_x, my_y, my_z);
    }
    
    // Create subcommunicators for each dimension
    MPI_Comm x_comm, y_comm, z_comm;
    int remain_dims[3];
    
    // X-plane communicator: vary X, fix Y and Z
    // All processes with same (Y, Z) but different X
    remain_dims[0] = 1;  // Include X dimension
    remain_dims[1] = 0;  // Exclude Y dimension
    remain_dims[2] = 0;  // Exclude Z dimension
    MPI_Cart_sub(cart_comm, remain_dims, &x_comm);
    
    // Y-plane communicator: vary Y, fix X and Z
    // All processes with same (X, Z) but different Y
    remain_dims[0] = 0;  // Exclude X dimension
    remain_dims[1] = 1;  // Include Y dimension
    remain_dims[2] = 0;  // Exclude Z dimension
    MPI_Cart_sub(cart_comm, remain_dims, &y_comm);
    
    // Z-plane communicator: vary Z, fix X and Y
    // All processes with same (X, Y) but different Z
    remain_dims[0] = 0;  // Exclude X dimension
    remain_dims[1] = 0;  // Exclude Y dimension
    remain_dims[2] = 1;  // Include Z dimension
    MPI_Cart_sub(cart_comm, remain_dims, &z_comm);
    
    // Allocate temporary buffers (need 2 for 3D)
    int type_size;
    MPI_Type_size(datatype, &type_size);
    void *tempbuf1 = malloc(count * type_size);
    void *tempbuf2 = malloc(count * type_size);
    
    if (!tempbuf1 || !tempbuf2) {
        fprintf(stderr, "Rank %d: Failed to allocate temp buffers\n", rank);
        MPI_Abort(comm, 1);
    }
    
    // Calculate chunk sizes for each phase
    int x_chunk = count / cbrtP;              // After X reduce-scatter
    int y_chunk = x_chunk / cbrtP;            // After Y reduce-scatter
    int z_chunk = y_chunk / cbrtP;            // After Z reduce-scatter = count/P
    
    if (rank == 0) {
        printf("Data distribution:\n");
        printf("  Initial: %d elements per process\n", count);
        printf("  After X reduce-scatter: %d elements per process\n", x_chunk);
        printf("  After Y reduce-scatter: %d elements per process\n", y_chunk);
        printf("  After Z reduce-scatter: %d elements per process (= count/P)\n", z_chunk);
    }
    
    // ========================================================================
    // REDUCE-SCATTER PHASE (3 steps)
    // ========================================================================
    
    // PHASE 1: Reduce-scatter along X dimension
    // Each YZ-plane reduces data along X
    MPI_Reduce_scatter_block(sendbuf, tempbuf1, x_chunk, datatype, op, x_comm);
    if (rank == 0) printf("Phase 1 complete: Reduce-scatter along X\n");
    
    // PHASE 2: Reduce-scatter along Y dimension
    // Each XZ-plane reduces data along Y
    MPI_Reduce_scatter_block(tempbuf1, tempbuf2, y_chunk, datatype, op, y_comm);
    if (rank == 0) printf("Phase 2 complete: Reduce-scatter along Y\n");
    
    // PHASE 3: Reduce-scatter along Z dimension
    // Each XY-plane reduces data along Z
    // After this, each process has count/P reduced elements
    MPI_Reduce_scatter_block(tempbuf2, recvbuf, z_chunk, datatype, op, z_comm);
    if (rank == 0) printf("Phase 3 complete: Reduce-scatter along Z\n");
    
    // ========================================================================
    // ALLGATHER PHASE (3 steps) - Reverse order
    // ========================================================================
    
    // PHASE 4: Allgather along Z dimension
    // Distribute data across Z dimension
    MPI_Allgather(recvbuf, z_chunk, datatype, tempbuf2, z_chunk, datatype, z_comm);
    if (rank == 0) printf("Phase 4 complete: Allgather along Z\n");
    
    // PHASE 5: Allgather along Y dimension
    // Distribute data across Y dimension
    MPI_Allgather(tempbuf2, y_chunk, datatype, tempbuf1, y_chunk, datatype, y_comm);
    if (rank == 0) printf("Phase 5 complete: Allgather along Y\n");
    
    // PHASE 6: Allgather along X dimension
    // Final distribution - all processes now have complete result
    MPI_Allgather(tempbuf1, x_chunk, datatype, recvbuf, x_chunk, datatype, x_comm);
    if (rank == 0) printf("Phase 6 complete: Allgather along X\n");
    
    // Cleanup
    free(tempbuf1);
    free(tempbuf2);
    MPI_Comm_free(&x_comm);
    MPI_Comm_free(&y_comm);
    MPI_Comm_free(&z_comm);
    MPI_Comm_free(&cart_comm);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Verify P is a perfect cube
    int cbrtP = (int)(pow(size, 1.0/3.0) + 0.5);
    if (cbrtP * cbrtP * cbrtP != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Number of processes (%d) must be a perfect cube\n", size);
            fprintf(stderr, "Try: 8, 27, 64, 125, 216, 343, 512, 729, 1000, ...\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Test parameters
    int count = 1024 * 1024;  // 1M elements = 4MB for integers
    int *sendbuf = (int*)malloc(count * sizeof(int));
    int *recvbuf_std = (int*)malloc(count * sizeof(int));
    int *recvbuf_3d = (int*)malloc(count * sizeof(int));
    
    // Initialize sendbuf: each process contributes its rank
    for (int i = 0; i < count; i++) {
        sendbuf[i] = rank;
    }
    
    if (rank == 0) {
        printf("\n========================================\n");
        printf("3D SUARA Allreduce Benchmark\n");
        printf("========================================\n");
        printf("Processes: %d (%dx%dx%d grid)\n", size, cbrtP, cbrtP, cbrtP);
        printf("Message size: %d integers (%.2f MB)\n", 
               count, count * sizeof(int) / (1024.0 * 1024.0));
        printf("Expected sum per element: %d\n", (size * (size - 1)) / 2);
        printf("========================================\n\n");
    }
    
    // Benchmark standard MPI_Allreduce
    MPI_Barrier(MPI_COMM_WORLD);
    double start_std = MPI_Wtime();
    MPI_Allreduce(sendbuf, recvbuf_std, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    double time_std = MPI_Wtime() - start_std;
    
    // Benchmark 3D SUARA
    MPI_Barrier(MPI_COMM_WORLD);
    double start_3d = MPI_Wtime();
    allreduce_3d_suara(sendbuf, recvbuf_3d, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    double time_3d = MPI_Wtime() - start_3d;
    
    // Verify correctness
    int errors = 0;
    int expected = (size * (size - 1)) / 2;  // Sum of ranks from 0 to size-1
    for (int i = 0; i < count && errors < 10; i++) {
        if (recvbuf_3d[i] != expected) {
            errors++;
            if (rank == 0) {
                fprintf(stderr, "Error at index %d: expected %d, got %d\n",
                        i, expected, recvbuf_3d[i]);
            }
        }
    }
    
    // Print results
    if (rank == 0) {
        printf("\n========================================\n");
        printf("Results:\n");
        printf("========================================\n");
        printf("Standard MPI_Allreduce: %.6f seconds\n", time_std);
        printf("3D SUARA Allreduce:     %.6f seconds\n", time_3d);
        printf("Speedup:                %.2fx\n", time_std / time_3d);
        printf("Theoretical speedup:    %.2fx (∛P = ∛%d)\n", 
               (int)(pow(size, 1.0/3.0) + 0.5), size);
        printf("========================================\n");
        
        if (errors == 0) {
            printf("✓ Correctness: PASSED\n");
        } else {
            printf("✗ Correctness: FAILED (%d errors found)\n", errors);
        }
        printf("========================================\n\n");
    }
    
    // Cleanup
    free(sendbuf);
    free(recvbuf_std);
    free(recvbuf_3d);
    
    MPI_Finalize();
    return 0;
}
