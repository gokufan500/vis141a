#include "gpu_renderer.h"
#include <glad/glad.h> 
#include <vector>
#include <cstring> 

namespace {
  unsigned fbo_ = 0, tex_ = 0;
  unsigned program_ = 0, vao_ = 0;
  int samples_ = 1;
  int cur_w_ = 0, cur_h_ = 0; 

  void ensure_framebuffer(int w, int h){
    if (w == cur_w_ && h == cur_h_ && fbo != 0) return;
    
    if (fbo_ == 0) glGenFramebuffers(1, &fbo_);
    if (tex_ == 0) glGenTextures(1, &tex_);

    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex_, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE){return;}
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    cur_w_ = w;
    cur_h_ = h;
  }

  struct CameraUniforms {
    float cx, cy, cz; 
    float px, py, pz;
    float ux, uy, uz; 
    float vx, vy, vz; 
  };

  CameraUniforms make_camera_uniforms(const TileInfo& tile){
    const float W = static_cast<float>(tile.global_width);
    const float H = static_cast<float>(tile.global_height);
    const float aspect = W / H; 

    const float viewport_h = 2.0f;
    const float viewport_w = viewport_h * aspect; 

    const float uvec[3] = { viewport_w, 0.0f, 0.0f };
    const float vvec[3] = { 0.0f, -viewport_h, 0.0f };
    const float du[3]   = { uvec[0] / W, uvec[1] / W, uvec[2] / W };
    const float dv[3]   = { vvec[0] / H, vvec[1] / H, vvec[2] / H };

    const float ul[3] = {
      ul[0] + 0.5f * (du[0] + dv[0]),
      ul[1] + 0.5f * (du[1] + dv[1]),
      ul[2] + 0.5f * (du[2] + dv[2]),
    };
    const float p00[3] = {
      ul[0] + 0.5f * (du[0] + dv[0]),
      ul[1] + 0.5f * (du[1] + dv[1]),
      ul[2] + 0.5f * (du[2] + dv[2]),
    };

    return {
      0.0f, 0.0f, 0.0f, 
      p00[0], p00[1], p00[2], 
      du[0], du[1], du[2],
      dv[0], dv[1], dv[2],
    };
  }
}

void gpu_renderer_init(){
  try {
    program_ = loadShaderProgram("shaders/raytrace.vert", "shaders/raytrace.frag");

  } catch (const std::exception& e) {
    std::fprintf(stderr, "gpu renderer init", e.what());
    program_ = 0;
    return; 
  }
  glGenVertexArrays(1, &vao_);
}
void gpu_renderer_shutdown();
void gpu_renderer_set_samples(int n) { samples_ = (n > 0 ? n : 1);}

void gpuRaytracer(uint8_t* out, const TileInfo& tile){

}
