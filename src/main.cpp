#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "cluster.h"
#include "renderer.h"
#include "scene.h"
#include "window_manager.h"

static void error_callback(int error, const char* description) {
    std::cerr << "glfw error " << error << " : " << description << '\n';
}

int main(int argc, char** argv) {
    ClusterContext cluster = ClusterContext::init(argc, argv);
    std::cout << "Hello from process " << cluster.rank
              << " of " << cluster.size << '\n';

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialise GLFW\n";
        cluster.finalize();
        return -1;
    }

    int exit_code = 0;
    {
        // Local monitor resolution drives the window size.
        GLFWmonitor* monitor     = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode  = glfwGetVideoMode(monitor);
        const int width  = mode->width;
        const int height = mode->height;

        WindowManager windowManager(width, height, "glfw default");
        GLFWwindow* window = windowManager.getWindow();

        // If any rank failed to get a window, everyone must bail out together,
        // otherwise the collective calls below will deadlock.
        if (!cluster.all_true(window != nullptr)) {
            glfwTerminate();
            cluster.finalize();
            return -1;
        }

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialise GLAD\n";
            glfwTerminate();
            cluster.finalize();
            return -1;
        }

        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        // Every rank now knows its tile size and its offset.
        cluster.share_tile_dimensions(fbWidth, fbHeight);

        TileInfo tile;
        tile.local_width   = fbWidth;
        tile.local_height  = fbHeight;
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

        Renderer renderer;
        if (!renderer.init(fbWidth, fbHeight,
                           "shaders/fullscreen.vert",
                           "shaders/fullscreen.frag")) {
            glfwTerminate();
            cluster.finalize();
            return -1;
        }

        RenderCallback scene_render = [](uint8_t* pixels, const TileInfo& t) {
            renderGradient(pixels, t);
        };

        while (!windowManager.shouldClose()) {
            windowManager.pollEvents();
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.renderFrame(scene_render, tile);

            windowManager.swapBuffers();

	    // keeps in sync
            cluster.barrier();
        }

        renderer.shutdown(); 
    }

    glfwTerminate();
    cluster.finalize();
    return exit_code;
}
