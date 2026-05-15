#define GLFW_INCLUDE_VULKAN
#include "VulkanRenderer.hpp"

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

void initWindow(const std::string& wName = "Test window", const int width = 800, const int height = 600) {

    // Inicia GLFW
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    glfwInit();

    // Set GLFW to Not work with OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main() {

    // Create a windows
    initWindow("teste");

    if (vulkanRenderer.init(window) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // loop until close
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        vulkanRenderer.draw();
    }

    vulkanRenderer.cleanup();

    // destroy GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}
