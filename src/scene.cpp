
#include "scene.h"
#include "rtweekend.h" 

#include <cstddef>
#include <iostream>
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"


using color = vec3;


color ray_color(const ray& r, hittable_list world){
  hit_record rec; 
  if (world.hit(r,0,infinity, rec)) {
    return 0.5 * (rec.normal + color(1,1,1));
  }
  
  vec3 unit_direction = unit_vector(r.direction());
  auto a = 0.5*(unit_direction.y() + 1.0);
  return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
}

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
    float image_width = tile.global_width;
    float image_height = tile.global_height;
    float aspect_ratio = image_width / image_height; 
    //std::cout << "Global width " << tile.global_height << " Global height " << tile.global_width << "\n";
    //std::cout << "Aspect ratio " << image_width / image_height << "\n";
    

    hittable_list world; 
    world.add(make_shared<sphere>(point3(0,0,-1), 0.5));
    world.add(make_shared<sphere>(point3(0,-100.5,-1), 100));

    // camera 
    float focal_length = 1.0;

    // virtual height 
    float viewport_height = 2.0;
    float viewport_width = viewport_height * aspect_ratio; 
    
    // we will use right handed coords 
    // y axis up x axis right -z axis forward 
    point3 camera_center = point3(0,0,0);

    // vector across top of viewport, down left side of viewport 
    vec3 viewport_u = vec3(viewport_width, 0, 0);
    vec3 viewport_v = vec3(0,-viewport_height, 0);

    // delta between pixels 
    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;
    
    // find location of top left pixel 
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
    auto pixel00_loc = viewport_upper_left +0.5 * (pixel_delta_u + pixel_delta_v);

    for (int y = 0; y < tile.local_height; ++y) {
        const int gy = tile.offset_y + y;
        for (int x = 0; x < tile.local_width; ++x) {
            const int gx = tile.offset_x + x;
            
            auto pixel_center = pixel00_loc + (gx * pixel_delta_u) + (gy * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center; 
            ray r(camera_center, ray_direction);
   
            color pixel_color = ray_color(r, world);
            
            const std::size_t idx =
                (static_cast<std::size_t>(y) * tile.local_width + x) * 4;
            
            pixels[idx + 0] = to_byte(static_cast<int>(255.999 * pixel_color[0]));
            pixels[idx + 1] = to_byte(static_cast<int>(255.999 * pixel_color[1]));
            pixels[idx + 2] = to_byte(static_cast<int>(255.999 * pixel_color[2]));
            pixels[idx + 3] = 255;
        }
    }
}
