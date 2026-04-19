#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct QueueFamilyIndices {
    int graphicsFamily = -1;     // Location of graphics Queue Family
    int presentationFamily = -1; // Location of Presentation Queue family

    // check if queue families are valid
    bool isValid() { return graphicsFamily >= 0 && presentationFamily >= 0; }
};

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;    // Surface properties, e.g. image size/extent
    std::vector<VkSurfaceFormatKHR> formats;         // Surface image formats, e.g. RGBA and size of each colour
    std::vector<VkPresentModeKHR> presentationModes; // How images should be presented to screen
};

struct SwapchainImage {
    VkImage image;
    VkImageView imageView;
};
