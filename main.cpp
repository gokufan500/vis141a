#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <GL/gl.h>
#include <iomanip>

#ifdef USE_MPI
  #include <mpi.h>
#endif

#include "window_manager.h"
#include "shader.h"

static void error_callback(int error, const char* description)
{
    std::cerr << "glfw error " << error << " : " << description << std::endl;
}

void draw(uint8_t* pixel, int width, int height) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x){
      size_t idx = (static_cast<size_t>(y) * width + x) * 4;
      pixel[idx + 0] = static_cast<uint8_t>(x * 255 / width);
      pixel[idx + 1] = static_cast<uint8_t>(y * 255 / height);
      pixel[idx + 2] = 128;
      pixel[idx + 3] = 255;
    }
  }
}

int main(int argc, char** argv) 
{
  #ifdef USE_MPI 
    MPI_Init(&argc, &argv);
    int rank, size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
  #else 
    int rank = 0, size = 1;
  #endif 
  std::cout << "Hello from process " << rank << " of " << size << "\n";


  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cerr << " Failed GLFW " << std::endl;
    return -1;
  }

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  int width = mode->width;
  int height = mode->height;

  try 
  {
    WindowManager windowManager(width , height , "glfw default");
    GLFWwindow* window = windowManager.getWindow();
    if (!window) {return 0 ;}
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cerr << "Failed to initialize glad" << std::endl;
      return -1;
    } 
   
    // we want framebuffer size since this might differ on high dpi displays 
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    // pixel buffer object
    GLuint pbo = 0; 
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(fbWidth) * fbHeight * 4, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    
    // texture 
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbWidth, fbHeight,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // we  create a quad of two triangles to cover the whole screen 
    // then we can run a fragment shader on each pixel 

    float quadVertices[] = {
      -1.0f, -1.0f,   1.0f, -1.0f,   1.0f,  1.0f,
      -1.0f, -1.0f,   1.0f,  1.0f,  -1.0f,  1.0f,
    };
    
    GLuint quadVAO = 0, quadVBO = 0;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    // shader 
    GLuint shaderProgram = loadShaderProgram("shaders/fullscreen.vert", "shaders/fullscreen.frag");
    GLint uTextureLoc = glGetUniformLocation(shaderProgram, "uTexture");

    while (!windowManager.shouldClose())
    {
      windowManager.pollEvents();
      glClear(GL_COLOR_BUFFER_BIT);

      // per frame drawing 
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
      void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
      if (ptr) {
         draw(static_cast<uint8_t*>(ptr), fbWidth, fbHeight);
         if (!glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER)) { std::cerr << "Failed to unmap PBO!" << std::endl;};
      }
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fbWidth, fbHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
     
      // draw full screen quad 
      glUseProgram(shaderProgram);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glUniform1i(uTextureLoc, 0);
      glBindVertexArray(quadVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      GLenum err = glGetError();
      if (err != GL_NO_ERROR) {
          std::cerr << "GL error after draw: " << err << std::endl;
      }

      glBindVertexArray(0);
      glUseProgram(0);

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      glBindTexture(GL_TEXTURE_2D,0);

      
      windowManager.swapBuffers();
    }
    
    // cleanup 
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &pbo);
  } 
  catch (const std::exception& e)
  {
    std::cerr << "Error" << e.what() << std::endl;
    return -1;
  }
   
  glfwTerminate();
 
  #ifdef USE_MPI
    MPI_Finalize();
  #endif
  return 0;
}


