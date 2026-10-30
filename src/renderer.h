#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <glad/glad.h>

struct TileInfo {
    int offset_x       = 0;   // offset of this tile in the global image
    int offset_y       = 0;
    int local_width    = 0;   // dimensions of this tile
    int local_height   = 0;
    int global_width   = 0;   // dimensions of the full tiled image
    int global_height  = 0;
    int global_offset_x= 0;
    int global_offset_y= 0;   // for shader (gradient continuity)
    int local_offset_x = 0;
    int local_offset_y = 0;    // for composite upload
    int rank           = 0;
    int num_processes  = 1;
};


class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(int fb_width, int fb_height,
              const std::string& vert_path,
              const std::string& frag_path);

    void uploadTile(const uint8_t* pixels, int tw, int th, int ox, int oy);
    void renderFrame(int fb_w, int fb_h);
    void shutdown();


private:
  GLuint program_ = 0, tex_ = 0, vao_ = 0;
  int w_ = 0, h_ = 0;
};
