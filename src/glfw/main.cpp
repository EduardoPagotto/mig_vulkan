#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/glm.hpp>
// #include <glm/mat4x4.hpp>
#include "VulkanRenderer.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

void initWindow(std::string wName = "Test window", const int width = 800, const int height = 600) {

    // Inicia GLFW
    glfwInit();
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

    // Set GLFW to Not work with OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main() {

    // Create a windows
    initWindow("teste", 800, 600);

    if (const int fail = vulkanRenderer.init(window) == EXIT_FAILURE) {
        return fail;
    }

    // loop until close
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // destroy GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}
