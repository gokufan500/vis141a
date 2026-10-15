#pragma once

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
    void finalize() const;
};
