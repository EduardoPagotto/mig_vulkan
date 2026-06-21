#pragma once

#ifdef SET_GLFW_ENABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <SDL3/SDL_vulkan.h>
#endif
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ce {

    class VWrapp {
      public:
#ifdef SET_GLFW_ENABLE
        explicit VWrapp(GLFWwindow* window) {
#else
        explicit VWrapp(SDL_Window* window) {
#endif
            this->window = window;
            init_device();
        }

        virtual ~VWrapp();

        VkDevice& getLogical() { return logicalDevice; }
        VkPhysicalDevice& getPhysical() { return physicalDevice; }
        VkQueue& getGraphicsQueue() { return graphicsQueue; }
        VkQueue& getPresentationQueue() { return presentationQueue; }
        VkSurfaceKHR& getSurface() { return surface; }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

      private:
        // Vulkan components
        // - Main
        VkInstance instance;
        VkDebugReportCallbackEXT callback;
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentationQueue;
        VkSurfaceKHR surface;

        bool validationEnabled = true;

#ifdef SET_GLFW_ENABLE
        GLFWwindow* window;
#else
        SDL_Window* window;
#endif

        inline static std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        inline static std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};

        // - Create functions
        void init_device();
        void createInstance();
        void createDebugCallback();
        void createSurface();
        void getNewPhysicalDevice();
        void createLogicalDevice();

        // utils
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        static bool CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
        static bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtentions);
        static bool CheckValidationLayerSupport();
    };
} // namespace ce
