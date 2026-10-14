#include "window_manager.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description)
{
  std::cerr << "glfw error " << error << " : " << description << std::endl;
}
WindowManager::WindowManager(int width, int height, const char* title) : width(width), height(height)
{

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
  glfwSetKeyCallback(window, keyCallback);
}


WindowManager::~WindowManager()
{
  if (window)
  {
    glfwDestroyWindow(window);
  }
}

bool WindowManager::shouldClose() const { return glfwWindowShouldClose(window); }
void WindowManager::pollEvents() const {  glfwPollEvents(); }
void WindowManager::swapBuffers() const {  glfwSwapBuffers(window); }
void WindowManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
  glViewport(0,0,width,height);
}

void WindowManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}


