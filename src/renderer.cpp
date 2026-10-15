#include "renderer.h"
#include "shader.h"   // shader methods 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

Renderer::~Renderer() {
    // Do NOT delete GL objects here: the context is usually gone by now.
    // Call shutdown() explicitly before the window is destroyed.
}

bool Renderer::init(int fb_width, int fb_height,
                    const std::string& vert_path,
                    const std::string& frag_path)
{
    fb_width_  = fb_width;
    fb_height_ = fb_height;

    // pixel buffer object 
    glGenBuffers(1, &pbo_);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
    glBufferData(GL_PIXEL_UNPACK_BUFFER,
                 static_cast<GLsizeiptr>(fb_width) * fb_height * 4,
                 nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // texture pbo uses 
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fb_width, fb_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // use two triangles to create a square over screen
    const float quad_vertices[] = {
        -1.0f, -1.0f,   1.0f, -1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f,   1.0f,  1.0f,  -1.0f, 1.0f,
    };
    glGenVertexArrays(1, &quad_vao_);
    glGenBuffers(1, &quad_vbo_);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // shader is simple just takes color from texture
    shader_program_ = loadShaderProgram(vert_path, frag_path);
    if (shader_program_ == 0) {
        std::cerr << "Failed to load shader program\n";
        return false;
    }
    u_texture_loc_ = glGetUniformLocation(shader_program_, "uTexture");
    return true;
}

void Renderer::shutdown() {
    if (shader_program_) glDeleteProgram(shader_program_);
    if (quad_vao_)       glDeleteVertexArrays(1, &quad_vao_);
    if (quad_vbo_)       glDeleteBuffers(1, &quad_vbo_);
    if (texture_)        glDeleteTextures(1, &texture_);
    if (pbo_)            glDeleteBuffers(1, &pbo_);
    shader_program_ = quad_vao_ = quad_vbo_ = texture_ = pbo_ = 0;
    u_texture_loc_ = -1;
}

void Renderer::renderFrame(const RenderCallback& render, const TileInfo& tile) {
    // fill pbo, this is all of our drawing logic in render
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
    if (void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY)) {
        render(static_cast<uint8_t*>(ptr), tile);
        if (!glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER)) {
            std::cerr << "Failed to unmap PBO!\n";
        }
    } else {
        std::cerr << "Failed to map PBO!\n";
    }

    // upload pbo to gpu 
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fb_width_, fb_height_,
                    GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // draw the quad
    glUseProgram(shader_program_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glUniform1i(u_texture_loc_, 0);
    glBindVertexArray(quad_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (GLenum err = glGetError(); err != GL_NO_ERROR) {
        std::cerr << "GL error after draw: " << err << '\n';
    }
}
