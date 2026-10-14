#pragma once 

#include <GLFW/glfw3.h>

class WindowManager 
{
  public:
    WindowManager(int width, int height, const char* title);
    ~WindowManager();

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    GLFWwindow* getWindow() const {return window;}


  private:
    GLFWwindow* window;
    int width;
    int height;
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

};

