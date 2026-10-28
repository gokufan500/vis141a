#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstring> 
#include <algorithm> 

#include "cluster.h"
#include "renderer.h"
#include "scene.h"
#include "window_manager.h"

static bool shared_prologue(ClusterContext& cluster);

static int run_display(ClusterContext& cluster);
static int run_compute(ClusterContext& cluster);

// source, source width, source height, display, width, height, ofsset x offset y 
static void blit_into(const uint8_t* src, int sw, int sh, 
                            uint8_t* dst, int dw, int dh, 
                            int ox, int oy) {
  const int rows = std::min(sh, dh - oy);
  const int cols = std::min(sw, dw - ox);

  if (rows <= 0 || cols <= 0) return;
  for (int y =0; y < rows; ++y) {
    std::memcpy(dst + ((oy + y) * dw + ox) * 4, src + (y*sw) * 4, static_cast<size_t>(cols) * 4);
  }

}

static void error_callback(int error, const char* description) {
    std::cerr << "glfw error " << error << " : " << description << '\n';
}

static void drawTile(uint8_t* pixels, const TileInfo& tile) {
  renderRankColor(pixels, tile);
  //raytracer(pixels,tile);
  //renderGradient(pixels,tile);
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
      tile_w = mode->width / cluster.grid_cols;
      tile_h = mode->height / cluster.grid_rows;  
    }
  }
  cluster.share_window_dims(cluster, tile_w, tile_h);
  cluster.share_tile_dimensions(tile_w, tile_h);
  return cluster.all_true(tile_w > 0 && tile_h > 0);
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
        
        // tile for sending to gpu 
        TileInfo frameTile = tile;
        frameTile.local_width  = cluster.global_width; 
        frameTile.local_height = cluster.global_height; 
        frameTile.offset_x = 0;
        frameTile.offset_y = 0;

        std::cout << "Rank " << cluster.rank
                  << "  tile offset=(" << tile.offset_x << "," << tile.offset_y << ")"
                  << "  tile=" << tile.local_width << "x" << tile.local_height
                  << "  global=" << tile.global_width << "x" << tile.global_height
                  << '\n';

        Renderer renderer;
        if (!renderer.init(fbWidth, fbHeight,
                           "shaders/fullscreen.vert",
                           "shaders/fullscreen.frag")) {
            return -1;
        }
        
        std::vector<uint8_t> myBuf(static_cast<size_t>(cluster.tile_width) * cluster.tile_height * 4);
        
        RenderCallback scene_render = [&](uint8_t* pixels, const TileInfo& t) {
            // rending our own tile 
            drawTile(myBuf.data(), tile);
            blit_into(myBuf.data(), cluster.tile_width, cluster.tile_height, 
                pixels, t.local_width, t.local_height, tile.offset_x, tile.offset_y);

            // get pixels from compute nodes 
            for (int src : cluster.compute_ranks_on_my_node()){
              std::vector<uint8_t> peerBuf; // their buffer 
              cluster.recieve_tile_pixels(src, 0 , peerBuf); 

              int px = (src % cluster.grid_cols) * cluster.tile_width; 
              int py = (src / cluster.grid_cols) * cluster.tile_height; 
              blit_into(peerBuf.data(), cluster.tile_width, cluster.tile_height, 
                  pixels, t.local_width, t.local_height, px, py);
            }
        };
        
        int running = 1;
        while (running) {
            running = !windowManager.shouldClose(); 
            // display process knows window liftime, so it tells compute process when to die 
            int flag = running; 
            cluster.broadcast_int(flag, 0);
            if (!flag) break; 

            windowManager.pollEvents();
            glClear(GL_COLOR_BUFFER_BIT);
            renderer.renderFrame(scene_render, frameTile);
            windowManager.swapBuffers();
            cluster.display_barrier();
        }

        renderer.shutdown(); 

    return 0;

}

static int run_compute(ClusterContext& cluster){
  
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) return -1;

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
  
  std::vector<uint8_t> pixels(static_cast<size_t>(tile.local_width) * tile.local_height * 4);

  int running = 1; 
  while (running) {
    cluster.broadcast_int(running, 0); 
    if (!running) break;
    drawTile(pixels.data(), tile);
    cluster.send_tile_pixels(pixels, 0);
  }
  return 0;
}
