#include <GLFW/glfw3.h>
#include <iostream>
#include <GL/gl.h>
#include <iomanip>

#include "window_manager.h"

static void error_callback(int error, const char* description)
{
    std::cerr << "glfw error " << error << " : " << description << std::endl;
}

int main() 
{
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
    
    if (!window)
    {
      return 0 ;
    }
    
    

    while (!windowManager.shouldClose())
    {
      windowManager.pollEvents();
      glClear(GL_COLOR_BUFFER_BIT);
      windowManager.swapBuffers();
    }

  } 
  catch (const std::exception& e)
  {
    std::cerr << "Error" << e.what() << std::endl;
    return -1;
  }
  glfwTerminate();
  return 0;
}


