#include "cluster.h"

#ifdef USE_MPI
  #include <mpi.h>
#endif

#include <cmath>

ClusterContext ClusterContext::init(int argc, char** argv) {
    ClusterContext ctx;

#ifdef USE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ctx.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ctx.size);
#else
    (void)argc; (void)argv;
#endif

    // Choose an approximately square grid for the process count.
    ctx.grid_cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(ctx.size))));
    ctx.grid_rows = (ctx.size + ctx.grid_cols - 1) / ctx.grid_cols;
    return ctx;
}

void ClusterContext::share_tile_dimensions(int local_w, int local_h) {
    int dims[2] = { local_w, local_h };
#ifdef USE_MPI
    MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
#endif
    tile_width    = dims[0];
    tile_height   = dims[1];
    global_width  = tile_width  * grid_cols;
    global_height = tile_height * grid_rows;
}

bool ClusterContext::all_true(bool local) const {
    int v = local ? 1 : 0;
#ifdef USE_MPI
    MPI_Allreduce(MPI_IN_PLACE, &v, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
#endif
    return v != 0;
}

void ClusterContext::barrier() const {
#ifdef USE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}

void ClusterContext::finalize() const {
#ifdef USE_MPI
    MPI_Finalize();
#endif
}
