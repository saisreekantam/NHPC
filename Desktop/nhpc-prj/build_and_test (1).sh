#!/bin/bash
#
# Build and Test Script for 2D/3D SUARA Allreduce
#
# This script provides options to:
#   1. Compile with regular MPI (for real cluster)
#   2. Compile with SimGrid (for simulation)
#   3. Run tests with different process counts
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored message
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Detect MPI compiler
detect_mpi() {
    if command_exists mpicc; then
        MPI_CC="mpicc"
    elif command_exists mpiicc; then
        MPI_CC="mpiicc"
    elif command_exists cc; then
        MPI_CC="cc"
    else
        print_error "No MPI compiler found!"
        print_info "Please install: sudo apt-get install openmpi-bin libopenmpi-dev"
        exit 1
    fi
    print_info "Using MPI compiler: $MPI_CC"
}

# Detect SimGrid
detect_simgrid() {
    if command_exists smpicc; then
        SMPI_CC="smpicc"
        print_info "SimGrid found: $SMPI_CC"
        return 0
    else
        print_warning "SimGrid not found"
        print_info "Install: sudo apt-get install simgrid"
        print_info "Or visit: https://simgrid.org/doc/latest/Installing_SimGrid.html"
        return 1
    fi
}

# Build with regular MPI
build_mpi() {
    print_info "Building with MPI..."
    detect_mpi
    
    $MPI_CC -o allreduce_2d_mpi allreduce_2d_complete.c -lm -O3 -Wall
    print_success "Built: allreduce_2d_mpi"
    
    $MPI_CC -o allreduce_3d_mpi allreduce_3d_complete.c -lm -O3 -Wall
    print_success "Built: allreduce_3d_mpi"
}

# Build with SimGrid
build_simgrid() {
    print_info "Building with SimGrid..."
    
    if ! detect_simgrid; then
        print_error "Cannot build SimGrid version without SimGrid installed"
        return 1
    fi
    
    $SMPI_CC -o allreduce_2d_simgrid allreduce_2d_complete.c -lm -O3 -Wall
    print_success "Built: allreduce_2d_simgrid"
    
    $SMPI_CC -o allreduce_3d_simgrid allreduce_3d_complete.c -lm -O3 -Wall
    print_success "Built: allreduce_3d_simgrid"
}

# Test 2D with MPI
test_2d_mpi() {
    local P=$1
    print_info "Testing 2D SUARA with P=$P processes (MPI)"
    
    # Check if P is perfect square
    sqrtP=$(echo "sqrt($P)" | bc)
    if [ $((sqrtP * sqrtP)) -ne $P ]; then
        print_error "P=$P is not a perfect square for 2D"
        return 1
    fi
    
    mpirun -np $P ./allreduce_2d_mpi
}

# Test 3D with MPI
test_3d_mpi() {
    local P=$1
    print_info "Testing 3D SUARA with P=$P processes (MPI)"
    
    # Check if P is perfect cube
    cbrtP=$(echo "e(l($P)/3)" | bc -l | xargs printf "%.0f")
    if [ $((cbrtP * cbrtP * cbrtP)) -ne $P ]; then
        print_error "P=$P is not a perfect cube for 3D"
        return 1
    fi
    
    mpirun -np $P ./allreduce_3d_mpi
}

# Test with SimGrid
test_simgrid() {
    local dim=$1
    local P=$2
    
    print_info "Testing ${dim}D SUARA with P=$P processes (SimGrid)"
    
    if [ ! -f "cluster.xml" ]; then
        print_error "cluster.xml not found. Create SimGrid platform file first."
        return 1
    fi
    
    # Create hostfile for SimGrid
    > hostfile.txt
    for ((i=0; i<P; i++)); do
        echo "node-$i.cluster.edu" >> hostfile.txt
    done
    
    smpirun -np $P \
            -platform cluster.xml \
            -hostfile hostfile.txt \
            ./allreduce_${dim}d_simgrid
}

# Run comprehensive tests
run_all_tests() {
    local mode=$1  # "mpi" or "simgrid"
    
    print_info "Running comprehensive tests with $mode"
    
    echo ""
    echo "========================================"
    echo "2D SUARA Tests"
    echo "========================================"
    
    # Test 2D with various process counts
    for P in 4 9 16 25 36 49 64; do
        echo ""
        if [ "$mode" = "mpi" ]; then
            test_2d_mpi $P || true
        else
            test_simgrid 2 $P || true
        fi
    done
    
    echo ""
    echo "========================================"
    echo "3D SUARA Tests"
    echo "========================================"
    
    # Test 3D with various process counts
    for P in 8 27 64; do
        echo ""
        if [ "$mode" = "mpi" ]; then
            test_3d_mpi $P || true
        else
            test_simgrid 3 $P || true
        fi
    done
}

# Show usage
usage() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build-mpi              Build with regular MPI"
    echo "  build-simgrid          Build with SimGrid"
    echo "  build-all              Build both versions"
    echo ""
    echo "  test-2d-mpi <P>        Test 2D with P processes (MPI)"
    echo "  test-3d-mpi <P>        Test 3D with P processes (MPI)"
    echo "  test-2d-simgrid <P>    Test 2D with P processes (SimGrid)"
    echo "  test-3d-simgrid <P>    Test 3D with P processes (SimGrid)"
    echo ""
    echo "  test-all-mpi           Run all tests with MPI"
    echo "  test-all-simgrid       Run all tests with SimGrid"
    echo ""
    echo "  clean                  Remove all build artifacts"
    echo ""
    echo "Examples:"
    echo "  $0 build-all"
    echo "  $0 test-2d-mpi 64"
    echo "  $0 test-3d-mpi 64"
    echo "  $0 test-all-mpi"
}

# Main script logic
main() {
    if [ $# -eq 0 ]; then
        usage
        exit 0
    fi
    
    case "$1" in
        build-mpi)
            build_mpi
            ;;
        build-simgrid)
            build_simgrid
            ;;
        build-all)
            build_mpi
            build_simgrid
            ;;
        test-2d-mpi)
            if [ -z "$2" ]; then
                print_error "Please specify number of processes"
                exit 1
            fi
            test_2d_mpi $2
            ;;
        test-3d-mpi)
            if [ -z "$2" ]; then
                print_error "Please specify number of processes"
                exit 1
            fi
            test_3d_mpi $2
            ;;
        test-2d-simgrid)
            if [ -z "$2" ]; then
                print_error "Please specify number of processes"
                exit 1
            fi
            test_simgrid 2 $2
            ;;
        test-3d-simgrid)
            if [ -z "$2" ]; then
                print_error "Please specify number of processes"
                exit 1
            fi
            test_simgrid 3 $2
            ;;
        test-all-mpi)
            run_all_tests mpi
            ;;
        test-all-simgrid)
            run_all_tests simgrid
            ;;
        clean)
            print_info "Cleaning build artifacts..."
            rm -f allreduce_2d_mpi allreduce_3d_mpi
            rm -f allreduce_2d_simgrid allreduce_3d_simgrid
            rm -f hostfile.txt
            print_success "Clean complete"
            ;;
        *)
            print_error "Unknown command: $1"
            usage
            exit 1
            ;;
    esac
}

main "$@"
