#pragma once 
#include <cstdint>
#include "renderer.h" 

void gpu_renderer_init();
void gpu_renderer_shutdown();
void gpu_renderer_set_samples( int n );

void gpuRaytracer(uint8_t* pixels, const TileInfo* tile);
