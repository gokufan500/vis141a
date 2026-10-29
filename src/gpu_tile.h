#pragma once 
#include <cstdint> 
#include <vector>
#include <glad/glad.h>
#include "scene.h"

class GpuTileRenderer {
  public: 
    GpuTileRenderer() = default;
    ~GpuTileRenderer();
    GpuTileRenderer(const GpuTileRenderer&) = delete; 
    GpuTileRenderer& operator=(const GpuTileRenderer&) = delete;

    bool init(int tile_w, int tile_h);
    void shutdown();

    void renderAndReadback(const TileInfo& tile, GLuint program, std::vector<uint8_t>& out);

  private:
    void renderOnly(const TileInfo& tile, GLuint program);

    GLuint fbo_ = 0, tex_ = 0, vao_ = 0;
    int w_ = 0, h_ = 0;
};
