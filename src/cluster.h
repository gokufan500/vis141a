#pragma once
#include <vector>
#include <cstdint>
#ifdef USE_MPI
  #include <mpi.h>
#endif

enum class Role { Display, Compute };
enum class Group { World, Display, Compute };

struct ClusterContext {
    int rank         = 0;
    int size         = 1;
    int display_rank = 0;   // index among display processes (grid position)
    int grid_cols    = 1;
    int grid_rows    = 1;

    int tile_width    = 0;
    int tile_height   = 0;
    int global_width  = 0;
    int global_height = 0;

    Role role       = Role::Compute;
    int  local_rank = 0;
    int  local_size = 1;

    int node_display_rank = 0;
    const std::vector<int>& peers_on_my_node() const {return peers_;}

#ifdef USE_MPI
    MPI_Comm display_comm = MPI_COMM_NULL;
    MPI_Comm compute_comm = MPI_COMM_NULL;
#endif

    static ClusterContext init(int argc, char** argv);

    int tile_col()      const { return grid_cols > 0 ? display_rank % grid_cols : 0; }
    int tile_row()      const { return grid_cols > 0 ? display_rank / grid_cols : 0; }
    int tile_offset_x() const { return tile_col() * tile_width; }
    int tile_offset_y() const { return tile_row() * tile_height; }

    void share_tile_dimensions(int local_w, int local_h);
    bool all_true(bool local) const;
    void broadcast_int(int& v, int root = 0) const;
    void barrier() const;
    void finalize() const;
  private: 
    std::vector<int> peers_;
};
