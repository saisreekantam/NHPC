/**
 * SimGrid-based 2D/3D Allreduce Simulation
 * 
 * This provides a middle ground between analytical simulation and real implementation:
 * - Uses actual MPI code (so implementation complexity is real)
 * - Simulates network behavior realistically
 * - No need for actual HPC cluster
 * 
 * Installation:
 *   sudo apt-get install simgrid
 *   Or: https://simgrid.org/doc/latest/Installing_SimGrid.html
 * 
 * Compilation:
 *   smpicc -o allreduce_simgrid allreduce_simgrid.c
 * 
 * Running:
 *   smpirun -np 64 -platform cluster.xml -hostfile hostfile.txt ./allreduce_simgrid
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define ROOT 0

/**
 * 2D SUARA Allreduce using actual MPI primitives
 */
void allreduce_2d_suara(void *sendbuf, void *recvbuf, int count, 
                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Calculate 2D grid dimensions
    int sqrtP = (int)sqrt(size);
    if (sqrtP * sqrtP != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Process count %d is not a perfect square\n", size);
        }
        MPI_Abort(comm, 1);
    }
    
    // Create 2D Cartesian topology
    int dims[2] = {sqrtP, sqrtP};
    int periods[2] = {0, 0};
    int reorder = 0;
    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 2, dims, periods, reorder, &cart_comm);
    
    // Create row and column subcommunicators
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
    
    // Allocate temporary buffer
    int type_size;
    MPI_Type_size(datatype, &type_size);
    void *tempbuf = malloc(count * type_size);
    
    // Phase 1: Reduce-scatter along rows
    int row_count = count / sqrtP;
    MPI_Reduce_scatter_block(sendbuf, tempbuf, row_count, datatype, op, row_comm);
    
    // Phase 2: Reduce-scatter along columns
    int col_count = row_count / sqrtP;
    MPI_Reduce_scatter_block(tempbuf, recvbuf, col_count, datatype, op, col_comm);
    
    // Phase 3: Allgather along columns
    MPI_Allgather(recvbuf, col_count, datatype, tempbuf, col_count, datatype, col_comm);
    
    // Phase 4: Allgather along rows
    MPI_Allgather(tempbuf, row_count, datatype, recvbuf, row_count, datatype, row_comm);
    
    // Cleanup
    free(tempbuf);
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    MPI_Comm_free(&cart_comm);
}

/**
 * 3D SUARA Allreduce - The "Hard" Implementation
 */
void allreduce_3d_suara(void *sendbuf, void *recvbuf, int count,
                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Calculate 3D grid dimensions
    int cbrtP = (int)(pow(size, 1.0/3.0) + 0.5);
    if (cbrtP * cbrtP * cbrtP != size) {
        if (rank == 0) {
            fprintf(stderr, "Error: Process count %d is not a perfect cube\n", size);
        }
        MPI_Abort(comm, 1);
    }
    
    // Create 3D Cartesian topology
    int dims[3] = {cbrtP, cbrtP, cbrtP};
    int periods[3] = {0, 0, 0};
    int reorder = 0;
    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 3, dims, periods, reorder, &cart_comm);
    
    // Create subcommunicators for each dimension
    MPI_Comm x_comm, y_comm, z_comm;
    int remain_dims[3];
    
    // X-plane communicator (vary x, fix y and z)
    remain_dims[0] = 1; remain_dims[1] = 0; remain_dims[2] = 0;
    MPI_Cart_sub(cart_comm, remain_dims, &x_comm);
    
    // Y-plane communicator (vary y, fix x and z)
    remain_dims[0] = 0; remain_dims[1] = 1; remain_dims[2] = 0;
    MPI_Cart_sub(cart_comm, remain_dims, &y_comm);
    
    // Z-plane communicator (vary z, fix x and y)
    remain_dims[0] = 0; remain_dims[1] = 0; remain_dims[2] = 1;
    MPI_Cart_sub(cart_comm, remain_dims, &z_comm);
    
    // Allocate temporary buffers
    int type_size;
    MPI_Type_size(datatype, &type_size);
    void *tempbuf1 = malloc(count * type_size);
    void *tempbuf2 = malloc(count * type_size);
    
    // Calculate chunk sizes for each dimension
    int x_count = count / cbrtP;
    int y_count = x_count / cbrtP;
    int z_count = y_count / cbrtP;
    
    // Phase 1: Reduce-scatter along X
    MPI_Reduce_scatter_block(sendbuf, tempbuf1, x_count, datatype, op, x_comm);
    
    // Phase 2: Reduce-scatter along Y
    MPI_Reduce_scatter_block(tempbuf1, tempbuf2, y_count, datatype, op, y_comm);
    
    // Phase 3: Reduce-scatter along Z
    MPI_Reduce_scatter_block(tempbuf2, recvbuf, z_count, datatype, op, z_comm);
    
    // Phase 4: Allgather along Z
    MPI_Allgather(recvbuf, z_count, datatype, tempbuf2, z_count, datatype, z_comm);
    
    // Phase 5: Allgather along Y
    MPI_Allgather(tempbuf2, y_count, datatype, tempbuf1, y_count, datatype, y_comm);
    
    // Phase 6: Allgather along X
    MPI_Allgather(tempbuf1, x_count, datatype, recvbuf, x_count, datatype, x_comm);
    
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
    
    // Test parameters
    int count = 1024 * 1024;  // 1M integers = 4MB
    int *sendbuf = (int*)malloc(count * sizeof(int));
    int *recvbuf = (int*)malloc(count * sizeof(int));
    
    // Initialize sendbuf
    for (int i = 0; i < count; i++) {
        sendbuf[i] = rank;
    }
    
    // Benchmark standard MPI_Allreduce
    MPI_Barrier(MPI_COMM_WORLD);
    double start_std = MPI_Wtime();
    MPI_Allreduce(sendbuf, recvbuf, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    double time_std = MPI_Wtime() - start_std;
    
    // Re-initialize for next test
    for (int i = 0; i < count; i++) {
        sendbuf[i] = rank;
        recvbuf[i] = 0;
    }
    
    // Benchmark 2D SUARA (if P is perfect square)
    int sqrtP = (int)sqrt(size);
    double time_2d = 0.0;
    if (sqrtP * sqrtP == size) {
        MPI_Barrier(MPI_COMM_WORLD);
        double start_2d = MPI_Wtime();
        allreduce_2d_suara(sendbuf, recvbuf, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        time_2d = MPI_Wtime() - start_2d;
    }
    
    // Re-initialize for next test
    for (int i = 0; i < count; i++) {
        sendbuf[i] = rank;
        recvbuf[i] = 0;
    }
    
    // Benchmark 3D SUARA (if P is perfect cube)
    int cbrtP = (int)(pow(size, 1.0/3.0) + 0.5);
    double time_3d = 0.0;
    if (cbrtP * cbrtP * cbrtP == size) {
        MPI_Barrier(MPI_COMM_WORLD);
        double start_3d = MPI_Wtime();
        allreduce_3d_suara(sendbuf, recvbuf, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        time_3d = MPI_Wtime() - start_3d;
    }
    
    // Print results from rank 0
    if (rank == 0) {
        printf("\n================================\n");
        printf("Allreduce Performance Comparison\n");
        printf("================================\n");
        printf("Processes: %d\n", size);
        printf("Message size: %d integers (%.2f MB)\n", count, count * sizeof(int) / (1024.0 * 1024.0));
        printf("\n");
        printf("Standard MPI_Allreduce: %.6f seconds\n", time_std);
        
        if (time_2d > 0) {
            printf("2D SUARA Allreduce:     %.6f seconds (%.2fx speedup)\n", 
                   time_2d, time_std / time_2d);
        }
        
        if (time_3d > 0) {
            printf("3D SUARA Allreduce:     %.6f seconds (%.2fx speedup)\n",
                   time_3d, time_std / time_3d);
        }
        printf("================================\n\n");
    }
    
    // Cleanup
    free(sendbuf);
    free(recvbuf);
    
    MPI_Finalize();
    return 0;
}
