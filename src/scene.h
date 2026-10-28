#pragma once

#include <cstdint>
#include "renderer.h"   // for TileInfo

void renderGradient(uint8_t* pixels, const TileInfo& tile);
void raytracer(uint8_t* pixels, const TileInfo& tile);
void renderRankColor(uint8_t* pixels, const TileInfo& tile);
