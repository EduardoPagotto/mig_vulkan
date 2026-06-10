#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "VWrapp.hpp"
#include "VulkanRenderer.hpp"
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>

GLFWwindow* window;

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

    int result = EXIT_SUCCESS;

    try {

        std::shared_ptr<ce::VWrapp> vwrapp = std::make_shared<ce::VWrapp>(window);
        std::shared_ptr<VulkanRenderer> vulkanRenderer = std::make_shared<VulkanRenderer>(vwrapp);

        float angle = 0.0F;
        float deltaTime = 0.0F;
        float lastTime = 0.0F;

        int helicopter = vulkanRenderer->createMeshModel("./models/Seahawk.obj");

        // loop until close
        while (glfwWindowShouldClose(window) == 0) {
            glfwPollEvents();

            float now = glfwGetTime();
            deltaTime = now - lastTime;
            lastTime = now;

            angle += 10.0F * deltaTime;
            if (angle > 360.0F) {
                angle -= 360.0F;
            }

            glm::mat4 testMat = glm::rotate(glm::mat4(1.0F), glm::radians(angle), glm::vec3(0.0F, 1.0F, 0.0F));
            //  testMat = glm::rotate(testMat, glm::radians(-45.0F), glm::vec3(0.0F, 0.0F, 1.0F));
            //  this->modelList[0].setModel(testMat);

            vulkanRenderer->updateModel(helicopter, testMat);
            vulkanRenderer->draw();
        }

    } catch (const std::runtime_error& e) {

        std::cout << "Error: " << e.what() << '\n';

        result = EXIT_FAILURE;
    }

    // destroy GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return result;
}
