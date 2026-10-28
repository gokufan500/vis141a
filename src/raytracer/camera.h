#ifndef CAMERA_H 
#define CAMERA_H 

#include "hittable.h" 

using color = vec3;


class camera {
  public: 

    float image_width = 0.0f;
    float image_height = 0.0f;
    float aspect_ratio = 1.0f; 
    int samples_per_pixel = 10;

    void render(const hittable& world, uint8_t* pixels, const TileInfo& tile) {
      initialize();
      
      for (int y = 0; y < tile.local_height; ++y) {
          const int gy = tile.offset_y + y;
          for (int x = 0; x < tile.local_width; ++x) {
              const int gx = tile.offset_x + x;
              
              auto pixel_center = pixel00_loc + (gx * pixel_delta_u) + (gy * pixel_delta_v);
              auto ray_direction = pixel_center - camera_center; 
              ray r(camera_center, ray_direction);
    
              color pixel_color = ray_color(r, world);
             
              for (int sample = 0; sample < samples_per_pixel; ++sample){
                ray r = get_ray(gx,gy);
                pixel_color += ray_color(r, world);
              }

              const std::size_t idx =
                  (static_cast<std::size_t>(y) * tile.local_width + x) * 4;
             
              pixel_color *= pixel_samples_scale;
              pixels[idx + 0] = to_byte(static_cast<int>(255.999 * pixel_color[0]));
              pixels[idx + 1] = to_byte(static_cast<int>(255.999 * pixel_color[1]));
              pixels[idx + 2] = to_byte(static_cast<int>(255.999 * pixel_color[2]));
              pixels[idx + 3] = 255;
          }
      }
    }
  private:
    point3 camera_center; 
    point3 pixel00_loc; 
    vec3   pixel_delta_u; 
    vec3   pixel_delta_v;
    double pixel_samples_scale; 
  
    void initialize() {
        
      pixel_samples_scale = 1.0 / samples_per_pixel;

      // camera 
      float focal_length = 1.0;

      // virtual height 
      float viewport_height = 2.0;
      float viewport_width = viewport_height * aspect_ratio; 
      
      // we will use right handed coords 
      // y axis up x axis right -z axis forward 
      camera_center = point3(0,0,0);

      // vector across top of viewport, down left side of viewport 
      auto viewport_u = vec3(viewport_width, 0, 0);
      auto viewport_v = vec3(0,-viewport_height, 0);

      // delta between pixels 
      pixel_delta_u = viewport_u / static_cast<double>(image_width);
      pixel_delta_v = viewport_v / static_cast<double>(image_height);
      
      // find location of top left pixel 
      auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
      pixel00_loc = viewport_upper_left +0.5 * (pixel_delta_u + pixel_delta_v);
    }
      
    ray get_ray(int i, int j) const {
      
      auto offset = sample_square(); 
      auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

      auto ray_origin = camera_center; 
      auto ray_direction = pixel_sample - ray_origin;

      return ray(ray_origin, ray_direction);
    }
    vec3 sample_square() const {
      return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    color ray_color(const ray& r, const hittable& world) const {
      hit_record rec; 
      if (world.hit(r, interval(0, infinity), rec)) {
        return 0.5 * (rec.normal + color(1,1,1));
      }
      
      vec3 unit_direction = unit_vector(r.direction());
      auto a = 0.5*(unit_direction.y() + 1.0);
      return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
    }

};

#endif 
