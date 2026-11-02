#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <glad/glad.h>

struct TileInfo {
    int tile_width     = 0;
    int tile_height    = 0;
    int offset_x       = 0;   // offset of this tile in the global image
    int offset_y       = 0;
    int global_width   = 0;   // dimensions of the full tiled image
    int global_height  = 0;
    int tile_col       = 0;
    int tile_row       = 0;
    int grid_cols      = 0;
    int grid_rows      = 0;
};

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Requires an active GL context. Loads shaders from the given paths.
    bool init(const std::string& vert_path, const std::string& frag_path);

    // Must be called while the GL context is still alive.
    void shutdown();

    void renderFrame(const TileInfo& tile);

private:
    GLuint shader_program_ = 0;
    GLuint quad_vao_       = 0;
    GLuint quad_vbo_       = 0;

    GLint u_tileOffset_ = -1;
    GLint u_tileSize_   = -1;
    GLint u_globalSize_ = -1;
    GLint u_tileIndex_  = -1;
    GLint u_gridSize_   = -1;
};
