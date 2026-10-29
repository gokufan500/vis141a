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
    int tile_w = 0, tile_h = 0;

    if (cluster.role == Role::Display) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            tile_w = mode->width  / cluster.grid_cols;
            tile_h = mode->height / cluster.grid_rows;
        }
    }

    cluster.share_window_dims(cluster, tile_w, tile_h);
    cluster.share_tile_dimensions(tile_w, tile_h);

    return cluster.all_true(tile_w > 0 && tile_h > 0);
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
        WindowManager windowManager(cluster.global_width, cluster.global_height, "glfw default");
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
        // tile display does processes for 
        TileInfo tile;
        tile.local_width   = cluster.tile_width;
        tile.local_height  = cluster.tile_height;
        tile.global_width  = cluster.global_width;
        tile.global_height = cluster.global_height;
        tile.offset_x      = cluster.tile_offset_x();
        tile.offset_y      = cluster.tile_offset_y();
        tile.rank          = cluster.rank;
        tile.num_processes = cluster.size;
        

        std::cout << "Rank " << cluster.rank
                  << "  tile offset=(" << tile.offset_x << "," << tile.offset_y << ")"
                  << "  tile=" << tile.local_width << "x" << tile.local_height
                  << "  global=" << tile.global_width << "x" << tile.global_height
                  << '\n';
        // composite.frag composites all tile into window 
        Renderer renderer;
        if (!renderer.init(cluster.global_width, cluster.global_height,
                           "shaders/fullscreen.vert",
                           "shaders/composite.frag")) {
            return -1;
        }

        // renders the individual tile 
        GpuTileRenderer gpu; 
        if (!gpu.init(tile.local_width, tile.local_height)) return -1;
        GLuint program = loadShaderProgram("shaders/fullscreen.vert", "shaders/render_tile.frag");

        std::vector<uint8_t> myBuf;

        int running = 1;
        while (running) {
            running = !windowManager.shouldClose();

            int flag = running; 
            cluster.broadcast_int(flag, 0);
            if (!flag) break; 

            windowManager.pollEvents();
            gpu.renderAndReadback(tile, program, myBuf);
            //size_t nz = 0;
            //for (size_t i = 0; i < myBuf.size(); i += 4)
             //   if (myBuf[i] | myBuf[i+1] | myBuf[i+2]) { nz++; if (nz > 5) break; }
            //std::cerr << "[display] myBuf=" << myBuf.size()
             //         << "B nonzero_rgb_samples=" << nz << "\n";
            renderer.uploadTile(myBuf.data(), tile.local_width, tile.local_height,
                                tile.offset_x, tile.offset_y);

            for (int src : cluster.compute_ranks_on_my_node()) {
                std::vector<uint8_t> peerBuf;
                cluster.recieve_tile_pixels(src, 0, peerBuf);
                int px = (src % cluster.grid_cols) * cluster.tile_width;
                int py = (src / cluster.grid_cols) * cluster.tile_height;
                renderer.uploadTile(peerBuf.data(),
                                    cluster.tile_width, cluster.tile_height,
                                    px, py);
            }

            glClear(GL_COLOR_BUFFER_BIT);
            renderer.renderFrame();
            windowManager.swapBuffers();
            cluster.display_barrier();
        }
        
    glDeleteProgram(program);
    gpu.shutdown();
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
