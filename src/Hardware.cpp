#include "Hardware.hpp"
#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>

namespace ce_new {

    Hardware::Hardware(const std::string& wName, const int& width, const int& height, bool validationEnabled)
        : validationEnabled(validationEnabled) {

#ifdef SET_GLFW_ENABLE
        // Inicia GLFW
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        glfwInit();

        // Set GLFW to Not work with OpenGL
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
#else
        // TODO:
#endif

        createInstance();
        createDebugCallback();
        createSurface();        // create before physical
        getNewPhysicalDevice(); // now need see if support surface
        createLogicalDevice();
    }

    void Hardware::createInstance() {

        if (this->validationEnabled && !Hardware::CheckValidationLayerSupport()) {
            throw std::runtime_error("Required Validation Layers not supported!");
        }

        // Information about the aplication itself
        // Most data here doesn't affect program and is for developer convinience
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Vulkan app Teste",         // Custom name of the aplication
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0), // Version app
            .pEngineName = "No engine",                     // Engine name
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),      // engine version
            .apiVersion = VK_API_VERSION_1_0                // the version of vulkan
        };

        // Create information for a VkInstance
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        // createInfo.pNext = nullptr;
        // createInfo.flags = VK_WHATEVER | WHAT_EVER2
        createInfo.pApplicationInfo = &appInfo;

        // Create a List to hold instance extencios
        std::vector<const char*> instanceExtensions = std::vector<const char*>();

        // set up extentions will use
        uint32_t hwExtentionCount = 0; // may require multiple extentions

#ifdef SET_GLFW_ENABLE
        const char** hwExtentions; // Extentions passed as array of cstring,
        ;                          // so need pointer (the array) to pointer(the string)
        // Get glfw extentions
        hwExtentions = glfwGetRequiredInstanceExtensions(&hwExtentionCount);
#else
        const char* const* hwExtentions; // Extentions passed as array of cstring,
        ;                                // so need pointer (the array) to pointer(the string)
        hwExtentions = SDL_Vulkan_GetInstanceExtensions(&hwExtentionCount);
#endif

        // Add glwf extentions to list of extentions
        for (size_t i = 0; i < hwExtentionCount; i++) {
            instanceExtensions.push_back(hwExtentions[i]);
        }

        // If validation enabled, add extension to report validation debug info
        if (this->validationEnabled) {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        // check Instance Extentions suppoted..
        if (!Hardware::CheckInstanceExtensionSupport(&instanceExtensions)) {
            throw std::runtime_error("vkInstance does no suport requerid extentions!");
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();

        // Set a validation layer tha instace will use
        if (validationEnabled) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(Hardware::validationLayers.size());
            createInfo.ppEnabledLayerNames = Hardware::validationLayers.data();

        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        // Create instance
        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan Instance");
        }
    }

    void Hardware::createDebugCallback() {
        // Only create callback if validation enabled
        if (!validationEnabled) {
            return;
        }

        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
        callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        callbackCreateInfo.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT; // Which validation reports should initiate callback
        callbackCreateInfo.pfnCallback = Hardware::DebugCallback;            // Pointer to callback function itself

        // Create debug callback with custom create function
        if (Hardware::CreateDebugReportCallbackEXT(this->instance, &callbackCreateInfo, nullptr, &Hardware::callback) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Debug Callback!");
        }
    }

    void Hardware::createSurface() {

        // Create Surface (creates a surface creste info struct, runs the create surface function, returns result)
#ifdef SET_GLFW_ENABLE
        VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &this->surface);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a surface!");
        }
#else
        bool result = SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
        if (!result) {
            throw std::runtime_error("Failed to create a surface!");
        }
#endif
    }

    void Hardware::getNewPhysicalDevice() {
        // Enumerate Physical devices the vkInstance can access
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // if no devices avaible, then none suport Vulkan!
        if (deviceCount == 0) {
            throw std::runtime_error("Cant find GPUs that support Vulkan Instance");
        }

        // get List of Physical devices
        std::vector<VkPhysicalDevice> deviceList(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

        // mainDevice.physicalDevice = deviceList[0];
        for (const auto& device : deviceList) {
            if (CheckDeviceSuitable(device, surface)) {
                this->physicalDevice = device;
                break;
            }
        }

        // Get properties of our new device
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(this->physicalDevice, &deviceProperties);

        // minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
    }

    void Hardware::createLogicalDevice() {

        // Get the queue family indices for the chosen Physical device
        QueueFamilyIndices indices = GetQueueFamilies(physicalDevice, surface);

        // vector for queue creation information, and set for family indices
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> queueFamilyIndices = {indices.graphicsFamily, indices.presentationFamily};

        // Queues the logical device needs to create and info to do so
        for (int queueFamiyIndex : queueFamilyIndices) {

            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamiyIndex; // The index of the family to create a from
            queueCreateInfo.queueCount = 1;                     // Numbers of queues to create
            float priority = 1.0F;                              //
            queueCreateInfo.pQueuePriorities =
                &priority; // Vulkan needs to know how to handle multiple queues, so decide priority (1 is hight)

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Information to create logical device (sometimes called "device")
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // Number queueCreateInfos
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data(); // List of queueCreateInfos so device can create required queues

        deviceCreateInfo.enabledExtensionCount =
            static_cast<uint32_t>(deviceExtensions.size());                 // Number of enable logical device extentions
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data(); // List of enable logical device extentions

        // Physical Device Features the Logical Device will be using
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE; // enable Anisotropy

        deviceCreateInfo.pEnabledFeatures = &deviceFeatures; // Physica device features logica device will use

        // Create the Logical device for the givem physical device
        VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a logical device");
        }

        // queues are created the same time as the device
        // so we want handle to queues
        // From given logical device, of given Queue Family, of given Queue Index(0 since only one), place reference in
        // given Vkqueue
        vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, indices.presentationFamily, 0, &presentationQueue);
    }

    VkExtent2D Hardware::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {

        // If current extend!!!!!!!!!!!!
        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return surfaceCapabilities.currentExtent;
        }

        int witdh;
        int height;
#ifdef SET_GLFW_ENABLE
        glfwGetFramebufferSize(window, &witdh, &height);
#else
        SDL_GetWindowSizeInPixels(window, &witdh, &height); // TODO: Testar
#endif
        VkExtent2D newExtent = {};
        newExtent.width = static_cast<uint32_t>(witdh);
        newExtent.height = static_cast<uint32_t>(height);

        // surface also defie max and min, so make sure within bondaries by clamping value
        newExtent.width =
            std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));

        newExtent.height =
            std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

        return newExtent;
    }

    bool Hardware::CheckValidationLayerSupport() {
        // Get number of validation layers to create vector of appropriate size
        uint32_t validationLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

        // Check if no validation layers found AND we want at least 1 layer
        if (validationLayerCount == 0 && Hardware::validationLayers.size() > 0) {
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
        for (const auto& validationLayer : Hardware::validationLayers) {
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

    bool Hardware::CheckInstanceExtensionSupport(std::vector<const char*>* checkExtentions) {
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

    void Hardware::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                 const VkAllocationCallbacks* pAllocator) {
        // get function pointer to requested function, then cast to function pointer for vkDestroyDebugReportCallbackEXT
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

        // If function found, execute
        if (func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Hardware::DebugCallback(VkDebugReportFlagsEXT flags,        // Type of error
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

    bool Hardware::CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {

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

    bool Hardware::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
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

    QueueFamilyIndices Hardware::GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

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

    VkFormat Hardware::ChooseSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& formats, VkImageTiling tilling,
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

    // --swapchain

    uint32_t Hardware::findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties) {
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
    VkSurfaceFormatKHR Hardware::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

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

    VkPresentModeKHR Hardware::ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes) {
        // Look for Mailbox presentation mode
        for (const auto& presentationMode : presentationModes) {
            if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentationMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // allways avaible by vulkan
    }

} // namespace ce_new
