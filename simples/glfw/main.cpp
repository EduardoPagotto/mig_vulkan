#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
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

    float angle = 0.0f;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    // loop until close
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();

        float now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        angle += 10.0f * deltaTime;
        if (angle > 360.0) {
            angle -= 360.0;
        }

        vulkanRenderer.updateModel(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)));

        vulkanRenderer.draw();
    }

    vulkanRenderer.cleanup();

    // destroy GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}
