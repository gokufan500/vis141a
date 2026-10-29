#include "gpu_tile.h"
#include <cstring>
#include <iostream>

GpuTileRenderer::~GpuTileRenderer() { shutdown(); }

bool GpuTileRenderer::init(int tile_w, int tile_h) {
  w_ = tile_w; h_ = tile_h; 

  glGenTextures(1, &tex_);
  glBindTexture(GL_TEXTURE_2D, tex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w_, h_, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, tex_, 0);
  GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (st != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "Tile FBO incomplete: 0x" << std::hex << st << '\n';
      return false;
  }

  glGenVertexArrays(1, &vao_);
  return true;
}

void GpuTileRenderer::shutdown() {
  if (fbo_) { glDeleteFramebuffers(1, &fbo_); fbo_ = 0; }
  if (tex_) { glDeleteTextures(1, &tex_);     tex_ = 0; }
  if (vao_) { glDeleteVertexArrays(1, &vao_); vao_ = 0; }
}

void GpuTileRenderer::renderOnly(const TileInfo& tile, GLuint program){
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, w_, h_);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);
  glUniform2f(glGetUniformLocation(program, "u_tile"),
              (float)tile.local_width, (float)tile.local_height);
  glUniform2f(glGetUniformLocation(program, "u_global"),
              (float)tile.global_width, (float)tile.global_height);
  glUniform2f(glGetUniformLocation(program, "u_offset"),
              (float)tile.offset_x, (float)tile.offset_y);
  glUniform1i(glGetUniformLocation(program, "u_rank"), tile.rank);

  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GpuTileRenderer::renderAndReadback(const TileInfo& tile, GLuint program, std::vector<uint8_t>& out){
  renderOnly(tile, program);
  out.resize(static_cast<size_t>(w_) * h_ * 4);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, w_, h_, GL_RGBA, GL_UNSIGNED_BYTE, out.data());
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
