#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstring> 
#include <algorithm> 

#include "cluster.h"
#include "renderer.h"
#include "window_manager.h"

static bool shared_prologue(ClusterContext& cluster);

static int run_display(ClusterContext& cluster);
static int run_compute(ClusterContext& cluster);

static void error_callback(int error, const char* description) {
    std::cerr << "glfw error " << error << " : " << description << '\n';
}

int main(int argc, char** argv) {
    ClusterContext cluster = ClusterContext::init(argc, argv);

    std::cout << "Hello from process " << cluster.rank
              << " of " << cluster.size
              << " role=" << (cluster.role == Role::Display ? "display" : "compute") 
              << '\n';
    // we do this on every process instead of in run display,
    // so that we can use glfw checks in shared setup
    if (cluster.role == Role::Display) {
      glfwSetErrorCallback(error_callback);
      if (!glfwInit()) {cluster.finalize();return -1; }
    }

    
    int exit_code = 0; 
    if (!shared_prologue(cluster)) {
      exit_code = -1; 
    } else {
      switch (cluster.role) {
        case Role::Display: exit_code = run_display(cluster); break; 
        case Role::Compute: exit_code = run_compute(cluster); break;
      }
    }
    if (cluster.role == Role::Display) glfwTerminate();
    cluster.finalize();

    return exit_code;
}


static bool shared_prologue(ClusterContext& cluster) {
  int tile_w = 0, tile_h = 0; 

  if (cluster.role == Role::Display) {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (mode) { 
      tile_w = mode->width;
      tile_h = mode->height;  
    }
  }
  cluster.share_tile_dimensions(tile_w, tile_h);
  return cluster.all_true(cluster.tile_width > 0 && cluster.tile_height > 0);
}


static int run_display(ClusterContext& cluster) {
        WindowManager wm(cluster.tile_width, cluster.tile_height, "video wall", /*fullscreen*/ false);
        GLFWwindow* window = wm.getWindow();
        if (!window) {
            return -1;
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialise GLAD\n";
            return -1;
        }

        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // tile display does processes for 
        TileInfo tile;
        tile.tile_width   = cluster.tile_width;
        tile.tile_height  = cluster.tile_height;
        tile.global_width  = cluster.global_width;
        tile.global_height = cluster.global_height;
        tile.offset_x      = cluster.tile_offset_x();
        tile.offset_y      = cluster.tile_offset_y();
        tile.tile_col      = cluster.tile_col();
        tile.tile_row      = cluster.tile_row();
        tile.grid_cols     = cluster.grid_cols;
        tile.grid_rows     = cluster.grid_rows;
        // tile for sending to gpu 

        std::cout << "Display rank " << cluster.rank
              << " (display_rank " << cluster.display_rank << ")"
              << " tile (" << tile.tile_col << "," << tile.tile_row << ")"
              << " offset=(" << tile.offset_x << "," << tile.offset_y << ")"
              << " tile=" << tile.tile_width << "x" << tile.tile_height
              << " global=" << tile.global_width << "x" << tile.global_height << '\n';
       
        Renderer renderer;
        if (!renderer.init(
                           "shaders/fullscreen.vert",
                           "shaders/fbm.frag")) {
            return -1;
        }
        //auto now = std::chrono::steady_clock::now().time_since_epoch();
        //float t = std::chrono::duration<float>(now).count();
        const float start_time = (float)glfwGetTime();

        int running = 1;
        while (running) {
            running = !wm.shouldClose(); 
            // display process knows window liftime, so it tells compute process when to die 
            int flag = running; 
            cluster.broadcast_int(flag, 0);
            if (!flag) break; 

            wm.pollEvents();
            glClear(GL_COLOR_BUFFER_BIT);

            const float t = (float)glfwGetTime() - start_time;

            renderer.renderFrame(tile, t);
            wm.swapBuffers();
        }

        renderer.shutdown(); 

    return 0;

}

static int run_compute(ClusterContext& cluster){
  int running = 1;
  while (running) {
    cluster.broadcast_int(running, 0);
    if (!running) break;
  }
  return 0;
}
