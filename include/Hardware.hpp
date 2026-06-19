#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#ifdef SET_GLFW_ENABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <SDL3/SDL_vulkan.h>
#endif

namespace ce_new {

    struct QueueFamilyIndices {
        int graphicsFamily = -1;     // Location of graphics Queue Family
        int presentationFamily = -1; // Location of Presentation Queue family

        // check if queue families are valid
        [[nodiscard]] bool isValid() const { return (graphicsFamily >= 0) && (presentationFamily >= 0); }
    };

    struct SwapChainDetails {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;    // Surface properties, e.g. image size/extent
        std::vector<VkSurfaceFormatKHR> formats;         // Surface image formats, e.g. RGBA and size of each colour
        std::vector<VkPresentModeKHR> presentationModes; // How images should be presented to screen
    };

    class Hardware {

      public:
        explicit Hardware(const std::string& wName = "Test window", const int& width = 800, const int& height = 600,
                          bool validationEnabled = false);
        virtual ~Hardware() = default;

        VkDevice& getLogical() { return logicalDevice; }
        VkPhysicalDevice& getPhysical() { return physicalDevice; }
        VkQueue& getGraphicsQueue() { return graphicsQueue; }
        VkQueue& getPresentationQueue() { return presentationQueue; }
        VkSurfaceKHR& getSurface() { return surface; }
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

      private:
#ifdef SET_GLFW_ENABLE
        GLFWwindow* window;
#else
        SDL_Window* window;
#endif
        VkInstance instance{VK_NULL_HANDLE};
        VkSurfaceKHR surface{VK_NULL_HANDLE};
        VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
        VkDevice logicalDevice{VK_NULL_HANDLE};
        VkQueue graphicsQueue{VK_NULL_HANDLE};
        VkQueue presentationQueue{VK_NULL_HANDLE};

        bool validationEnabled{false};

        inline static std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        inline static std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

        void createInstance();
        void createDebugCallback();
        void createSurface();
        void getNewPhysicalDevice();
        void createLogicalDevice();

        static VkDebugReportCallbackEXT callback;
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,        // Type of error
                                                            VkDebugReportObjectTypeEXT objType, // Type of object causing error
                                                            uint64_t obj,                       // ID of object
                                                            size_t location, int32_t code, const char* layerPrefix,
                                                            const char* message, // Validation Information
                                                            void* userData);

        static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
        static bool CheckValidationLayerSupport();
        static bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtentions);
        static bool CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
        static QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        static SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface);
    };
} // namespace ce_new
