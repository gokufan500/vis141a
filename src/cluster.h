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
    
    int wall_cols = 1; 
    int wall_rows = 1; 
    int monitor_width = 0;
    int monitor_height = 0; 
    int node_cols = 1;
    int node_rows = 1; 

    int tile_width    = monitor_width / node_cols;
    int tile_height   = monitor_height / node_rows;
    int global_width  = wall_cols * monitor_width;
    int global_height = wall_rows * monitor_height; 

    int node_wall_x = 0, node_wall_y = 0;   // this node's monitor in the wall
    const std::vector<int>& all_compute_ranks() const { return all_compute_ranks_; }
const std::vector<int>& all_display_ranks() const { return all_display_ranks_; }
// members:
    // Per-compute: tile position within this monitor
    int tile_lx = 0, tile_ly = 0;

    int tile_global_offset_x() const {
        return (node_wall_x * node_cols + tile_lx) * tile_width;
    }
    int tile_global_offset_y() const {
        return (node_wall_y * node_rows + tile_ly) * tile_height;
    }
    int tile_local_offset_x() const { return tile_lx * tile_width;  }
    int tile_local_offset_y() const { return tile_ly * tile_height; }
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
    std::vector<int> all_compute_ranks_;
    std::vector<int> all_display_ranks_;
};


