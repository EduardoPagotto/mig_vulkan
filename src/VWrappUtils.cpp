#include "VWrappUtils.hpp"
#include <cstring>
#include <iostream>

namespace ce {

    SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainDetails swapChainDetails;

        // -- CAPABILITIES --
        // Get the surface capabilities for the given surface on the given physical device
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);

        // -- FORMATS --
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        // If formats returned, get list of formats
        if (formatCount != 0) {
            swapChainDetails.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
        }

        // -- PRESENTATION MODES --
        uint32_t presentationCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

        // If presentation modes returned, get list of presentation modes
        if (presentationCount != 0) {
            swapChainDetails.presentationModes.resize(presentationCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
        }

        return swapChainDetails;
    }

    VkFormat ChooseSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& formats, VkImageTiling tilling,
                                   VkFormatFeatureFlags featureFlags) {

        // Loop through options and find compatible one
        for (VkFormat format : formats) {

            // Get properties for give format on this device
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(device, format, &properties);

            // Depending on tiling choice, nned to check for difference bit flag
            if (tilling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags) {
                //
                return format;
            }
            if (tilling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags) {
                //
                return format;
            }
        }

        throw std::runtime_error("Failed to find a matching format!");
    }

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

        QueueFamilyIndices indices;

        // Get all Queue Family Property info for the given device
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

        // Go through each queue family and check if it has at least 1 of the requered types of queue
        int idx = 0;
        for (const auto& queueFamily : queueFamilyList) {

            // First check if queue has at least 1 queue in that family (could have no queue)
            // Queue cam be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if
            // has requered type
            if ((queueFamily.queueCount > 0) && ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)) {
                indices.graphicsFamily = idx; // if queue family is valid then get index
            }

            // Check if Queue Family support presentation
            VkBool32 presentationSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface,
                                                 &presentationSupport); // TODO: validar se result OK
            // check if queue is presentation type (can bo boyh graphics and presentation)
            if ((queueFamily.queueCount > 0) && (presentationSupport == VK_TRUE)) {
                indices.presentationFamily = idx;
            }

            // check if queue family indices are in valid state, stop searching if so
            if (indices.isValid()) {
                break;
            }

            idx++;
        }

        return indices;
    }

    // --swapchain

    uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties) {
        // get properties of physical device memory
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

            // Index of memory type must match corresponding bit in allowedTypes and desired property bit flag are part of memory type's
            // property flags
            if ((allowedTypes & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) { // NOLINT
                // this memory type is valid, so return its index
                return i;
            }
        }

        throw std::runtime_error("Failed to find Memory!");
    }

    // Best format is subjective, but ours will be:
    // Format     : VK_FORMAT_R8G8B8A8_UNFORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
    // colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

        // If only 1 format avaible and is undefined, them this means ALL formats ase avaible (no restricion)
        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            return {.format = VK_FORMAT_R8G8B8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        // If restriced, searche for optimal format
        for (const auto& format : formats) {
            if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        // If can't find optimal format, then just return first format
        return formats[0]; // FIXME: pade data pau aqui
    }

    VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes) {
        // Look for Mailbox presentation mode
        for (const auto& presentationMode : presentationModes) {
            if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentationMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // allways avaible by vulkan
    }

} // namespace ce
