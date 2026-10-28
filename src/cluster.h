#pragma once
#include <vector> 
#include <cstdint> 
#ifdef USE_MPI 
  #include <mpi.h>
#endif

enum class Role {
  Display,
  Compute
};

enum class Group {
  World, Display, Compute
};


// all mpi logic, made so if we arent compiling with mpi still runs
struct ClusterContext {
    int rank       = 0;
    int size       = 1;
    int grid_cols  = 1;
    int grid_rows  = 1;

    int tile_width    = 0;
    int tile_height   = 0;
    int global_width  = 0;
    int global_height = 0;

    static ClusterContext init(int argc, char** argv);

    int tile_offset_x() const { return (rank % grid_cols) * tile_width; }
    int tile_offset_y() const { return (rank / grid_cols) * tile_height; }

    // rank 0 framebuffer size becomes basis for all others
    void share_tile_dimensions(int local_w, int local_h);

    bool all_true(bool local) const;

    void barrier() const;
    void display_barrier() const; 
    void compute_barrier() const; 
    void finalize() const;
    void broadcast_int(int& v, int root = 0) const; 

    Role role = Role::Compute;

    #ifdef USE_MPI
    MPI_Comm display_comm = MPI_COMM_NULL;
    MPI_Comm compute_comm = MPI_COMM_NULL;
    #endif 
    int local_rank = 0;
    int local_size = 1;
    int node_display_rank = 0;
    
    void share_window_dims(ClusterContext& ctx, int& w, int& h);
    bool probe_window_ok(ClusterContext& ctx, bool local_ok);

    void send_tile_pixels(const std::vector<uint8_t>& pixels, int tag) const; 
    void recieve_tile_pixels(int src, int tag, std::vector<uint8_t>& out) const; 

    const std::vector<int>& compute_ranks_on_my_node() const {return peers_;}
    bool have_compute_peers() const { return !peers_.empty(); }
  private: 
    std::vector<int> peers_;
};


