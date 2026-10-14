#include "window_manager.h"
#include <iostream>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description)
{
  std::cerr << "glfw error " << error << " : " << description << std::endl;
}
WindowManager::WindowManager(int width, int height, const char* title) : width(width), height(height)
{

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("Failed to init GLFW");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (!window)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create window");
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

}


WindowManager::~WindowManager()
{
  if (window)
  {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
}

bool WindowManager::shouldClose() const 
{
  return glfwWindowShouldClose(window);
}

void WindowManager::pollEvents() const 
{
  glfwPollEvents();
  if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
  {
  }
}

void WindowManager::swapBuffers() const 
{
  glfwSwapBuffers(window);
}
void WindowManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
  handleFrameBufferResize(window, width, height);
}

void WindowManager::handleFrameBufferResize(GLFWwindow* window, int width, int height) 
{
  glViewport(0,0,width,height);
}

