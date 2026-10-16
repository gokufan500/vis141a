#include "scene.h"

#include <cstddef>

namespace {
inline uint8_t to_byte(int v) {
    return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
}
}

void renderGradient(uint8_t* pixels, const TileInfo& tile) {
    for (int y = 0; y < tile.local_height; ++y) {
        const int gy = tile.offset_y + y;
        for (int x = 0; x < tile.local_width; ++x) {
            const int gx = tile.offset_x + x;
            const std::size_t idx =
                (static_cast<std::size_t>(y) * tile.local_width + x) * 4;

            pixels[idx + 0] = to_byte(gx * 255 / tile.global_width);
            pixels[idx + 1] = to_byte(gy * 255 / tile.global_height);
            pixels[idx + 2] = 128;
            pixels[idx + 3] = 255;
        }
    }
}

void raytracer(uint8_t* pixels, const TileInfo& tile) {

}
