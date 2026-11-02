#include "renderer.h"
#include "shader.h"   // shader methods 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

Renderer::~Renderer() {
    // Do NOT delete GL objects here: the context is usually gone by now.
    // Call shutdown() explicitly before the window is destroyed.
}

bool Renderer::init(
                    const std::string& vert_path,
                    const std::string& frag_path)
{
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

    shader_program_ = loadShaderProgram(vert_path, frag_path);
    if (shader_program_ == 0) {
        std::cerr << "Failed to load shader program\n";
        return false;
    }
    u_tileOffset_ = glGetUniformLocation(shader_program_, "uTileOffset");
    u_tileSize_   = glGetUniformLocation(shader_program_, "uTileSize");
    u_globalSize_ = glGetUniformLocation(shader_program_, "uGlobalSize");
    u_tileIndex_  = glGetUniformLocation(shader_program_, "uTileIndex");
    u_gridSize_   = glGetUniformLocation(shader_program_, "uGridSize");
    return true;
}

void Renderer::shutdown() {
    if (shader_program_) glDeleteProgram(shader_program_);
    if (quad_vao_)       glDeleteVertexArrays(1, &quad_vao_);
    if (quad_vbo_)       glDeleteBuffers(1, &quad_vbo_);
    shader_program_ = quad_vao_ = quad_vbo_ = 0;
}

void Renderer::renderFrame(const TileInfo& tile) {
    glUseProgram(shader_program_);
    glUniform2f(u_tileOffset_, (float)tile.offset_x,     (float)tile.offset_y);
    glUniform2f(u_tileSize_,   (float)tile.tile_width,   (float)tile.tile_height);
    glUniform2f(u_globalSize_, (float)tile.global_width, (float)tile.global_height);
    glUniform2f(u_tileIndex_,  (float)tile.tile_col,     (float)tile.tile_row);
    glUniform2f(u_gridSize_,   (float)tile.grid_cols,    (float)tile.grid_rows);

    glBindVertexArray(quad_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);

    if (GLenum err = glGetError(); err != GL_NO_ERROR) {
        std::cerr << "GL error after draw: " << err << '\n';
    }
}
