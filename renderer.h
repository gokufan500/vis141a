#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct TileInfo {
    int offset_x       = 0;   // offset of this tile in the global image
    int offset_y       = 0;
    int local_width    = 0;   // dimensions of this tile
    int local_height   = 0;
    int global_width   = 0;   // dimensions of the full tiled image
    int global_height  = 0;
    int rank           = 0;
    int num_processes  = 1;
};

// The scene renderer writes RGBA pixels for one tile into `pixels`
// (row-major, local_width * local_height * 4 bytes).
using RenderCallback = std::function<void(uint8_t* pixels, const TileInfo& tile)>;

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Requires an active GL context. Loads shaders from the given paths.
    bool init(int fb_width, int fb_height,
              const std::string& vert_path,
              const std::string& frag_path);

    // Must be called while the GL context is still alive.
    void shutdown();

    void renderFrame(const RenderCallback& render, const TileInfo& tile);

private:
    unsigned int pbo_            = 0;
    unsigned int texture_        = 0;
    unsigned int quad_vao_       = 0;
    unsigned int quad_vbo_       = 0;
    unsigned int shader_program_ = 0;
    int          u_texture_loc_  = -1;
    int          fb_width_       = 0;
    int          fb_height_      = 0;
};
