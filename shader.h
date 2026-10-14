#pragma once 
#include <glad/glad.h>
#include <string>

std::string readFile(const std::string& path);

GLuint compileShader(GLenum type, const std::string& source, const std::string& debugName);

GLuint loadShaderProgram(const std::string& vertPath, const std::string& fragPath);
