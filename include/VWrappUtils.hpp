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

    inline const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    inline const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

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

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);

    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,        // Type of error
                                                 VkDebugReportObjectTypeEXT objType, // Type of object causing error
                                                 uint64_t obj,                       // ID of object
                                                 size_t location, int32_t code, const char* layerPrefix,
                                                 const char* message, // Validation Information
                                                 void* userData);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtentions);

    bool CheckValidationLayerSupport();

    bool CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkFormat ChooseSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& formats, VkImageTiling tilling,
                                   VkFormatFeatureFlags featureFlags);

    SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    // -- Swapchain

    void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
                      VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

    uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties);

    VkImage createImage(VkPhysicalDevice physical, VkDevice device, uint32_t with, uint32_t height, VkFormat format, VkImageTiling tiling,
                        VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory);

    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes);

} // namespace ce
