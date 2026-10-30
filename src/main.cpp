#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstring> 
#include <algorithm> 

#include "cluster.h"
#include "renderer.h"
#include "scene.h"
#include "window_manager.h"
#include "gpu_tile.h"
#include "shader.h"

static int run_display(ClusterContext& cluster);
static int run_compute(ClusterContext& cluster);

// source, source width, source height, display, width, height, ofsset x offset y 

static void error_callback(int error, const char* description) {
    std::cerr << "glfw error " << error << " : " << description << '\n';
}
static bool shared_prologue(ClusterContext& cluster) {
    int mon_w = 0, mon_h = 0;

    if (cluster.rank == 0) {           // display 0 queries its monitor
        GLFWmonitor* mon  = glfwGetPrimaryMonitor();
        const GLFWvidmode* m = glfwGetVideoMode(mon);
        if (m) { mon_w = m->width; mon_h = m->height; }
    }
    cluster.broadcast_int(mon_w, 0);
    cluster.broadcast_int(mon_h, 0);
    if (mon_w <= 0 || mon_h <= 0) return false;

    cluster.monitor_width  = mon_w;
    cluster.monitor_height = mon_h;

    cluster.global_width  = cluster.wall_cols  * mon_w;
    cluster.global_height = cluster.wall_rows  * mon_h;
    cluster.tile_width    = mon_w / cluster.node_cols;
    cluster.tile_height   = mon_h / cluster.node_rows;

    return true;
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
    if (!shared_prologue(cluster)){
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




static int run_display(ClusterContext& cluster) {
        WindowManager windowManager(cluster.monitor_width, cluster.monitor_height, "glfw default");
        GLFWwindow* window = windowManager.getWindow();
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

        std::cerr << "[display] fb=" << fbWidth << "x" << fbHeight
          << "  win=" << cluster.global_width << "x" << cluster.global_height
          << "  tile=" << cluster.tile_width << "x" << cluster.tile_height << '\n';

        // composite.frag composites all tile into window 
        Renderer renderer;
        if (!renderer.init(cluster.monitor_width, cluster.monitor_height,
                           "shaders/fullscreen.vert",
                           "shaders/composite.frag")) {
            return -1;
        }

        int running = 1;
        while (running) {
            running = !windowManager.shouldClose();

            int flag = running; 
            cluster.broadcast_int(flag, 0);
            if (!flag) break; 

            windowManager.pollEvents();
          const auto& peers = cluster.compute_ranks_on_my_node();
          for (size_t i = 0; i < peers.size(); ++i) {
              int src = peers[i];
              std::vector<uint8_t> buf;
              cluster.recieve_tile_pixels(src, 0, buf);

              // local tile index i → (lx, ly) within THIS monitor
              int lx = (int)i % cluster.node_cols;
              int ly = (int)i / cluster.node_cols;
              int px = lx * cluster.tile_width;
              int py = ly * cluster.tile_height;

              renderer.uploadTile(buf.data(),
                                  cluster.tile_width, cluster.tile_height,
                                  px, py);   // LOCAL offsets
          }
            glViewport(0,0,fbWidth,fbHeight);
            glClear(GL_COLOR_BUFFER_BIT);
            renderer.renderFrame(fbWidth,fbHeight);
            windowManager.swapBuffers();
            cluster.display_barrier();
        }
    renderer.shutdown(); 
    return 0;

}

static int run_compute(ClusterContext& cluster){
  
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) return -1;
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
 
  GLFWwindow* window = glfwCreateWindow(64, 64, "offscreen", nullptr, nullptr);
  if (!window) { glfwTerminate(); return -1; }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  TileInfo tile;
  tile.local_width   = cluster.tile_width;
  tile.local_height  = cluster.tile_height;
  tile.global_width  = cluster.global_width;
  tile.global_height = cluster.global_height;
  tile.offset_x      = cluster.tile_offset_x();
  tile.offset_y      = cluster.tile_offset_y();
  tile.global_offset_x = cluster.tile_global_offset_x();   // for shader
  tile.global_offset_y = cluster.tile_global_offset_y();
  tile.rank          = cluster.rank;
  tile.num_processes = cluster.size;
  
  GpuTileRenderer gpu;
    if (!gpu.init(tile.local_width, tile.local_height)) return -1;
    GLuint program = loadShaderProgram("shaders/fullscreen.vert", "shaders/render_tile.frag");

    std::vector<uint8_t> pixels;
  int running = 1; 
  while (running) {
    cluster.broadcast_int(running, 0); 
    if (!running) break;
    gpu.renderAndReadback(tile, program, pixels);
    cluster.send_tile_pixels(pixels, 0);
  }
      glDeleteProgram(program);
    gpu.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
  return 0;
}
