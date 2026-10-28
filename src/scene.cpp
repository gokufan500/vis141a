
#include "scene.h"

#include <cstddef>
#include <iostream>
#include "raytracer/rtweekend.h" 
#include "raytracer/hittable.h"
#include "raytracer/hittable_list.h"
#include "raytracer/sphere.h"
#include "raytracer/camera.h"



void raytracer(uint8_t* pixels, const TileInfo& tile){
  hittable_list world; 
  world.add(make_shared<sphere>(point3(0,0,-1), 0.5));
  world.add(make_shared<sphere>(point3(0,-100.5,-1), 100));
    
  camera cam;
  cam.image_width  = static_cast<float>(tile.global_width);
  cam.image_height = static_cast<float>(tile.global_height); 
  cam.aspect_ratio = cam.image_width / cam.image_height;

  cam.samples_per_pixel = 10;
    
  cam.render(world, pixels, tile);

}

void renderRankColor(uint8_t* pixels, const TileInfo& tile) {
    // 16 visually distinct colors (from the "Kelly" / Sasha Trubetskoy palette).
    static const uint8_t kPalette[][3] = {
        {230,  25,  75}, { 60, 180,  75}, {255, 225,  25}, { 67,  99, 216},
        {245, 130,  49}, {145,  30, 180}, { 70, 240, 240}, {240,  50, 230},
        {210, 245,  60}, { 60,  60, 250}, {250, 190, 190}, {  0, 128, 128},
        {170, 110,  40}, {128,   0,   0}, {128, 128,   0}, {  0,   0, 128},
    };
    constexpr int kNumColors = sizeof(kPalette) / sizeof(kPalette[0]);
    const uint8_t* c = kPalette[tile.rank % kNumColors];

    for (int y = 0; y < tile.local_height; ++y) {
        for (int x = 0; x < tile.local_width; ++x) {
            const std::size_t idx =
                (static_cast<std::size_t>(y) * tile.local_width + x) * 4;

                pixels[idx + 0] = c[0];
                pixels[idx + 1] = c[1];
                pixels[idx + 2] = c[2];
            pixels[idx + 3] = 255;
        }
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

