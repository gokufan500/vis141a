#include <GLFW/glfw3.h>
#include <iostream>
#include <GL/gl.h>
#include <iomanip>

#include "window_manager.h"

int width = 800;

int height = 600;


int main() 
{
  try 
  {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    WindowManager windowManager(mode->width , mode->height , "glfw default");
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
  return 0;
}


