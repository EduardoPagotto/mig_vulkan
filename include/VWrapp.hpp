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

    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

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
        [[nodiscard]] VkFormat chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tilling, VkFormatFeatureFlags featureFlags);

        static SwapChainDetails getSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface);
        static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

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

        // - Create functions
        void init_device();
        void createInstance();
        void createDebugCallback();
        void createSurface();
        void getNewPhysicalDevice();
        void createLogicalDevice();

        bool static checkDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool static checkValidationLayerSupport(); // TODO: passar para PascalCase todos os estaticos depois de implementar em VulkanRender
        bool static checkInstanceExtensionSupport(std::vector<const char*>* checkExtentions);
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,        // Type of error
                                                            VkDebugReportObjectTypeEXT objType, // Type of object causing error
                                                            uint64_t obj,                       // ID of object
                                                            size_t location, int32_t code, const char* layerPrefix,
                                                            const char* message, // Validation Information
                                                            void* userData);
        static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
        static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    };
} // namespace ce
