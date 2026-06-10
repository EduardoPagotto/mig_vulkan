#include "VWrappUtils.hpp"
#include <cstring>
#include <iostream>

namespace ce {

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
        // vkGetInstanceProcAddr returns a function pointer to the requested function in the requested instance
        // resulting function is cast as a function pointer with the header of "vkCreateDebugReportCallbackEXT"
        auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

        // If function was found, executre if with given data and return result, otherwise, return error
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
        // get function pointer to requested function, then cast to function pointer for vkDestroyDebugReportCallbackEXT
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

        // If function found, execute
        if (func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,        // Type of error
                                                 VkDebugReportObjectTypeEXT objType, // Type of object causing error
                                                 uint64_t obj,                       // ID of object
                                                 size_t location, int32_t code, const char* layerPrefix,
                                                 const char* message, // Validation Information
                                                 void* userData) {
        // If validation ERROR, then output error and return failure
        if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
            std::cout << "VALIDATION ERROR: " << message << '\n';
            return VK_TRUE;
        }

        // If validation WARNING, then output warning and return okay
        if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0) {
            std::cout << "VALIDATION WARNING: " << message << '\n';
            return VK_FALSE;
        }

        return VK_FALSE;
    }

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

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
        // Get device extension count
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        // If no extensions found, return failure
        if (extensionCount == 0) {
            return false;
        }

        // Populate list of extensions
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

        // Check for extension
        for (const auto& deviceExtension : deviceExtensions) {
            bool hasExtension = false;
            for (const auto& extension : extensions) {
                if (std::strcmp(deviceExtension, extension.extensionName) == 0) {
                    hasExtension = true;
                    break;
                }
            }

            if (!hasExtension) {
                return false;
            }
        }

        return true;
    }

    bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtentions) {
        // need to get number of extentions to create array of correct size to hold extentions
        uint32_t extentionsCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extentionsCount, nullptr);

        // Create list of vKExtentionsProperties using count
        std::vector<VkExtensionProperties> extentions(extentionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extentionsCount, extentions.data());

        // check if give extentions are list of avaible extentins
        for (const auto& checkExtention : *checkExtentions) {
            bool hasExtentions = false;
            for (const auto& extention : extentions) {
                if (std::strcmp(checkExtention, extention.extensionName) == 0) {
                    std::cout << "Extenções: " << checkExtention << '\n';
                    hasExtentions = true;
                    break;
                }
            }

            if (!hasExtentions) {
                return false;
            }
        }

        return true;
    }

    bool CheckValidationLayerSupport() {
        // Get number of validation layers to create vector of appropriate size
        uint32_t validationLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

        // Check if no validation layers found AND we want at least 1 layer
        if (validationLayerCount == 0 && validationLayers.size() > 0) {
            return false;
        }

        std::vector<VkLayerProperties> availableLayers(validationLayerCount);
        vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

        std::cout << "Camadas Vulkan Disponiveis (" << validationLayerCount << "):" << '\n';
        for (const auto& layerProperties : availableLayers) {
            std::cout << "\tLayer Name: " << layerProperties.layerName << '\n';
            std::cout << "\tDescription: " << layerProperties.description << '\n';
            std::cout << "\tImplementation Version: " << layerProperties.implementationVersion << '\n';
            std::cout << "\tSpec Version: " << layerProperties.specVersion << '\n';
            std::cout << "\t-----------------------------------" << '\n';
        }

        // Check if given Validation Layer is in list of given Validation Layers
        for (const auto& validationLayer : validationLayers) {
            bool hasLayer = false;
            for (const auto& availableLayer : availableLayers) {
                if (std::strcmp(validationLayer, availableLayer.layerName) == 0) {
                    hasLayer = true;
                    break;
                }
            }

            if (!hasLayer) {
                return false;
            }
        }

        return true;
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

    bool CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {

        /*
        // Information abaout the device itself (ID, Name, Type Vendor, etc)
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        */
        // information about what the device can do (geo, Shader, tess, shader, wide lines, etc)
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        QueueFamilyIndices indices = GetQueueFamilies(device, surface);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainValid = false;
        if (extensionsSupported) {
            SwapChainDetails swapChainDetails = GetSwapChainDetails(device, surface);
            swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
        }

        return indices.isValid() && extensionsSupported && swapChainValid && (deviceFeatures.samplerAnisotropy == VK_TRUE);
    }
} // namespace ce
