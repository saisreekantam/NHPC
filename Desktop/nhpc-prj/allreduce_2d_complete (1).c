/**
 * 2D SUARA Allreduce Implementation
 * 
 * Process arrangement: √P × √P grid
 * Algorithm: 4 phases
 *   1. Reduce-scatter along rows
 *   2. Reduce-scatter along columns
 *   3. Allgather along columns
 *   4. Allgather along rows
 * 
 * Complexity:
 *   Steps: 4(√P - 1) for Ring, 2×log₂(P) for Rabenseifner
 *   Time: O(√P × α + M/√P × β)
 *   Speedup: √P over 1D
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

void allreduce_2d_suara(void *sendbuf, void *recvbuf, int count,
                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Verify P is a perfect square
    int sqrtP = (int)sqrt(size);
    if (sqrtP * sqrtP != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: P=%d must be perfect square for 2D SUARA\n", size);
            fprintf(stderr, "Valid values: 4, 9, 16, 25, 36, 49, 64, 81, 100, ...\n");
        }
        MPI_Abort(comm, 1);
    }
    
    if (rank == 0) {
        printf("2D SUARA: Arranging %d processes in %dx%d grid\n", size, sqrtP, sqrtP);
    }
    
    // Create 2D Cartesian topology
    // dims[0] = rows, dims[1] = columns
    int dims[2] = {sqrtP, sqrtP};
    int periods[2] = {0, 0};  // Non-periodic (no wrap-around)
    int reorder = 0;           // Don't allow MPI to reorder ranks
    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 2, dims, periods, reorder, &cart_comm);
    
    // Get my coordinates in the 2D grid
    int coords[2];
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    int my_row = coords[0];
    int my_col = coords[1];
    
    if (rank == 0) {
        printf("Example: Rank 0 is at grid position (%d, %d)\n", my_row, my_col);
    }
    
    // Create subcommunicators
    MPI_Comm row_comm, col_comm;
    int remain_dims[2];
    
    // Row communicator: all processes in the same row
    // (vary column index, fix row index)
    remain_dims[0] = 0;  // Don't include row dimension
    remain_dims[1] = 1;  // Include column dimension
    MPI_Cart_sub(cart_comm, remain_dims, &row_comm);
    
    // Column communicator: all processes in the same column
    // (vary row index, fix column index)
    remain_dims[0] = 1;  // Include row dimension
    remain_dims[1] = 0;  // Don't include column dimension
    MPI_Cart_sub(cart_comm, remain_dims, &col_comm);
    
    // Allocate temporary buffer
    int type_size;
    MPI_Type_size(datatype, &type_size);
    void *tempbuf = malloc(count * type_size);
    if (!tempbuf) {
        fprintf(stderr, "Rank %d: Failed to allocate temp buffer\n", rank);
        MPI_Abort(comm, 1);
    }
    
    // Calculate chunk sizes
    // After reduce-scatter along rows, each process has count/sqrtP elements
    // After reduce-scatter along cols, each process has count/(sqrtP×sqrtP) = count/P elements
    int row_chunk = count / sqrtP;
    int final_chunk = row_chunk / sqrtP;
    
    if (rank == 0) {
        printf("Data distribution:\n");
        printf("  Initial: %d elements per process\n", count);
        printf("  After row reduce-scatter: %d elements per process\n", row_chunk);
        printf("  After col reduce-scatter: %d elements per process\n", final_chunk);
    }
    
    // ========================================================================
    // PHASE 1: Reduce-scatter along rows
    // ========================================================================
    // Each process sends count/sqrtP elements to each process in its row
    // After this, each process has reduced data for its column partition
    MPI_Reduce_scatter_block(sendbuf, tempbuf, row_chunk, datatype, op, row_comm);
    
    if (rank == 0) printf("Phase 1 complete: Reduce-scatter along rows\n");
    
    // ========================================================================
    // PHASE 2: Reduce-scatter along columns
    // ========================================================================
    // Each process sends row_chunk/sqrtP elements to each process in its column
    // After this, each process has the final reduced result for its partition
    MPI_Reduce_scatter_block(tempbuf, recvbuf, final_chunk, datatype, op, col_comm);
    
    if (rank == 0) printf("Phase 2 complete: Reduce-scatter along columns\n");
    
    // ========================================================================
    // PHASE 3: Allgather along columns
    // ========================================================================
    // Gather the reduced data along each column
    // After this, each process has the complete reduced data for its row partition
    MPI_Allgather(recvbuf, final_chunk, datatype, tempbuf, final_chunk, datatype, col_comm);
    
    if (rank == 0) printf("Phase 3 complete: Allgather along columns\n");
    
    // ========================================================================
    // PHASE 4: Allgather along rows
    // ========================================================================
    // Gather the reduced data along each row
    // After this, all processes have the complete allreduce result
    MPI_Allgather(tempbuf, row_chunk, datatype, recvbuf, row_chunk, datatype, row_comm);
    
    if (rank == 0) printf("Phase 4 complete: Allgather along rows\n");
    
    // Cleanup
    free(tempbuf);
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    MPI_Comm_free(&cart_comm);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Verify P is a perfect square
    int sqrtP = (int)sqrt(size);
    if (sqrtP * sqrtP != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Number of processes (%d) must be a perfect square\n", size);
            fprintf(stderr, "Try: 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, ...\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Test parameters
    int count = 1024 * 1024;  // 1M elements = 4MB for integers
    int *sendbuf = (int*)malloc(count * sizeof(int));
    int *recvbuf_std = (int*)malloc(count * sizeof(int));
    int *recvbuf_2d = (int*)malloc(count * sizeof(int));
    
    // Initialize sendbuf: each process contributes its rank
    for (int i = 0; i < count; i++) {
        sendbuf[i] = rank;
    }
    
    if (rank == 0) {
        printf("\n========================================\n");
        printf("2D SUARA Allreduce Benchmark\n");
        printf("========================================\n");
        printf("Processes: %d (%dx%d grid)\n", size, sqrtP, sqrtP);
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
    
    // Benchmark 2D SUARA
    MPI_Barrier(MPI_COMM_WORLD);
    double start_2d = MPI_Wtime();
    allreduce_2d_suara(sendbuf, recvbuf_2d, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    double time_2d = MPI_Wtime() - start_2d;
    
    // Verify correctness
    int errors = 0;
    int expected = (size * (size - 1)) / 2;  // Sum of ranks from 0 to size-1
    for (int i = 0; i < count && errors < 10; i++) {
        if (recvbuf_2d[i] != expected) {
            errors++;
            if (rank == 0) {
                fprintf(stderr, "Error at index %d: expected %d, got %d\n",
                        i, expected, recvbuf_2d[i]);
            }
        }
    }
    
    // Print results
    if (rank == 0) {
        printf("\n========================================\n");
        printf("Results:\n");
        printf("========================================\n");
        printf("Standard MPI_Allreduce: %.6f seconds\n", time_std);
        printf("2D SUARA Allreduce:     %.6f seconds\n", time_2d);
        printf("Speedup:                %.2fx\n", time_std / time_2d);
        printf("Theoretical speedup:    %.2fx (√P = √%d)\n", sqrt(size), size);
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
    free(recvbuf_2d);
    
    MPI_Finalize();
    return 0;
}
