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

    // assigning jobs based on process 
    MPI_Comm node_comm;

    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &node_comm);
    MPI_Comm_rank(node_comm, &ctx.local_rank);
    MPI_Comm_size(node_comm, &ctx.local_size);
    
    // local rank 0 on each machine becomes in charge of display 
    // the rest are for compute 
    ctx.role = (ctx.local_rank == 0) ? Role::Display : Role::Compute;
    int color = (ctx.role == Role::Display) ? 0 : 1;

    MPI_Comm_split(MPI_COMM_WORLD, color, ctx.rank, (ctx.role == Role::Display) ? &ctx.display_comm : &ctx.compute_comm);
    
    if (ctx.role == Role::Display) {
      MPI_Comm_rank(ctx.display_comm, &ctx.display_rank);
    }

    int my_disp = (ctx.role == Role::Display) ? 1 : 0;
    int display_count = 0;
    MPI_Allreduce(&my_disp, &display_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    ctx.grid_cols = static_cast<int>(std::ceil(std::sqrt((double)display_count)));
    ctx.grid_rows = (display_count + ctx.grid_cols - 1) / ctx.grid_cols;

    (void) argc; (void) argv; 
    ctx.role = Role::Display;

#else
    (void)argc; (void)argv;
    ctx.role = Role::Display;
#endif

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


void ClusterContext::broadcast_int(int& v, int root) const {
#ifdef USE_MPI
  MPI_Bcast(&v, 1, MPI_INT, root, MPI_COMM_WORLD);
#else 
  (void)root;
#endif 
}
