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
    
    int display_local_rank = 0; 
    MPI_Bcast(&display_local_rank, 1, MPI_INT, 0, node_comm);

    int node_first_world_rank = MPI_PROC_NULL; 
    MPI_Allreduce(&ctx.rank, &node_first_world_rank, 1, MPI_INT, MPI_MIN, node_comm);
    ctx.node_display_rank = node_first_world_rank + display_local_rank;

    // collect world ranks of local ranks except display 
    std::vector<int> all_local(ctx.local_size); 
    MPI_Allgather(&ctx.rank, 1, MPI_INT, all_local.data(), 1, MPI_INT, node_comm);
    ctx.peers_.clear(); 
    for (int w : all_local) if (w != ctx.node_display_rank) ctx.peers_.push_back(w);

#else
    (void)argc; (void)argv;
    ctx.role = Role::Display;
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
void ClusterContext::display_barrier() const {
#ifdef USE_MPI
  if (role == Role::Display) { MPI_Barrier(display_comm);}
#endif
}
void ClusterContext::compute_barrier() const {
#ifdef USE_MPI
  if (role == Role::Compute) { MPI_Barrier(compute_comm);}
#endif
}
void ClusterContext::finalize() const {
#ifdef USE_MPI
    MPI_Finalize();
#endif
}

void ClusterContext::share_window_dims(ClusterContext& ctx, int& w, int& h){
#ifdef USE_MPI
  int dims[2] = { w, h };
  MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
  w = dims[0]; h = dims[1];
#endif 
}

bool ClusterContext::probe_window_ok(ClusterContext& ctx, bool local_ok){
  return ctx.all_true(local_ok);
}

void ClusterContext::send_tile_pixels( const std::vector<uint8_t>& pixels, int tag) const {
#ifdef USE_MPI 
  MPI_Send(pixels.data(), static_cast<int>(pixels.size()), MPI_BYTE, node_display_rank, tag, MPI_COMM_WORLD);

#else 
  (void)pixels; (void)tag;
#endif 
}

void ClusterContext::recieve_tile_pixels(int src, int tag, std::vector<uint8_t>& out) const {
#ifdef USE_MPI
  MPI_Status st; 
  MPI_Probe(src, tag, MPI_COMM_WORLD, &st);
  int n = 0; 
  MPI_Get_count(&st, MPI_BYTE, &n); 
  out.resize(static_cast<size_t>(n));
  MPI_Recv(out.data(), n, MPI_BYTE, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#else 
  (void)src; (void)tag; (void)out;
#endif 
}

void ClusterContext::broadcast_int(int& v, int root) const {
#ifdef USE_MPI
  MPI_Bcast(&v, 1, MPI_INT, root, MPI_COMM_WORLD);
#endif 
}
