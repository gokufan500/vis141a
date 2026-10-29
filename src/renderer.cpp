#include "renderer.h"
#include "shader.h"   // shader methods 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

Renderer::~Renderer() {
  shutdown();
}

bool Renderer::init(int fb_width, int fb_height,
                    const std::string& vert_path,
                    const std::string& frag_path)
{
    w_  = fb_width;
    h_ = fb_height;

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w_, h_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    {
        std::vector<uint8_t> zeros(static_cast<size_t>(w_) * h_ * 4, 0);
        for (size_t i = 3; i < zeros.size(); i += 4) zeros[i] = 255;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w_, h_,
                        GL_RGBA, GL_UNSIGNED_BYTE, zeros.data());
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // shader is simple just takes color from texture
      try {
        program_ = loadShaderProgram(vert_path, frag_path);
    } catch (const std::exception& e) {
        std::cerr << "Renderer::init shader error: " << e.what() << '\n';
        return false;
    }

    // Bind sampler once; it refers to GL_TEXTURE0 for the program's lifetime.
    glUseProgram(program_);
    glUniform1i(glGetUniformLocation(program_, "u_screen"), 0);
    glUseProgram(0);

    // --- VAO for the gl_VertexID fullscreen triangle -----------------------
    glGenVertexArrays(1, &vao_);
    
    return true;
}

void Renderer::uploadTile(const uint8_t* pixels, int tw, int th, int ox, int oy) {
    glBindTexture(GL_TEXTURE_2D, tex_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // ox/oy are top-left origin; GL texture origin is bottom-left.
    int yTex = h_ - oy - th;
   while (glGetError() != GL_NO_ERROR) {} 
    glTexSubImage2D(GL_TEXTURE_2D, 0, ox, yTex, tw, th,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
        std::cerr << "uploadTile(" << ox << "," << yTex
                  << " " << tw << "x" << th
                  << ") err=0x" << std::hex << e << std::dec
                  << "  fb=" << w_ << "x" << h_ << "\n";
}

void Renderer::renderFrame() {
glViewport(0, 0, w_, h_);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program_);
    glBindVertexArray(vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glUniform1i(glGetUniformLocation(program_, "u_screen"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void Renderer::shutdown() {
if (program_) { glDeleteProgram(program_);      program_ = 0; }
if (tex_)     { glDeleteTextures(1, &tex_);     tex_     = 0; }
if (vao_)     { glDeleteVertexArrays(1, &vao_); vao_     = 0; }
}
