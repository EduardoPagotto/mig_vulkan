#include "VulkanRenderer.hpp"
#include "Ultilities.hpp"
#include "VulkanValidation.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <vector>

VulkanRenderer::VulkanRenderer() {}

VulkanRenderer::~VulkanRenderer() {}

int VulkanRenderer::init(GLFWwindow* window) {
    this->window = window;

    try {
        this->createInstance();
        this->createDebugCallback();
        this->createSurface();     // create before physical
        this->getPhysicalDevice(); // now need see if support surface
        this->createLogicalDevices();
    } catch (const std::runtime_error& e) {
        printf("Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::cleanup() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);

    if (validationEnabled) {
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::createInstance() {

    if (validationEnabled && !checkValidationLayerSupport()) {
        throw std::runtime_error("Required Validation Layers not supported!");
    }

    // Information about the aplication itself
    // Most data here doesn't affect program and is for developer convinience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan app Teste";         // Custom name of the aplication
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version app
    appInfo.pEngineName = "No engine";                     // Engine name
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      // engine version
    appInfo.apiVersion = VK_API_VERSION_1_0;               // the version of vulkan

    // Create information for a VkInstance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // createInfo.pNext = nullptr;
    // createInfo.flags = VK_WHATEVER | WHAT_EVER2
    createInfo.pApplicationInfo = &appInfo;

    // Create a List to hold instance extencios
    std::vector<const char*> instanceExtensions = std::vector<const char*>();

    // set up extentions will use
    uint32_t glfwExtentionCount = 0; // GLFW may require multiple extentions
    const char** glfwExtentions;     // Extentions passed as array of cstring,
    ;                                // so need pointer (the array) to pointer(the string)
    // Get glfw extentions
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    // Add glwf extentions to list of extentions
    for (size_t i = 0; i < glfwExtentionCount; i++) {
        instanceExtensions.push_back(glfwExtentions[i]);
    }

    // If validation enabled, add extension to report validation debug info
    if (validationEnabled) {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // check Instance Extentions suppoted..
    if (!this->checkInstanceExtentionsSupport(&instanceExtensions)) {
        throw std::runtime_error("vkInstance does no suport requerid extentions!");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    // Set a validation layer tha instace will use
    if (validationEnabled) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    // Create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
    }
}

void VulkanRenderer::createDebugCallback() {
    // Only create callback if validation enabled
    if (!validationEnabled)
        return;

    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                               VK_DEBUG_REPORT_WARNING_BIT_EXT; // Which validation reports should initiate callback
    callbackCreateInfo.pfnCallback = debugCallback;             // Pointer to callback function itself

    // Create debug callback with custom create function
    VkResult result = CreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Debug Callback!");
    }
}

bool VulkanRenderer::checkInstanceExtentionsSupport(std::vector<const char*>* checkExtentions) {
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
            if (strcmp(checkExtention, extention.extensionName)) {
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

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
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
            if (strcmp(deviceExtension, extension.extensionName) == 0) {
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

bool VulkanRenderer::checkValidationLayerSupport() {
    // Get number of validation layers to create vector of appropriate size
    uint32_t validationLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

    // Check if no validation layers found AND we want at least 1 layer
    if (validationLayerCount == 0 && validationLayers.size() > 0) {
        return false;
    }

    std::vector<VkLayerProperties> availableLayers(validationLayerCount);
    vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

    std::cout << "Camadas Vulkan Disponiveis (" << validationLayerCount << "):" << std::endl;
    for (const auto& layerProperties : availableLayers) {
        std::cout << "\tLayer Name: " << layerProperties.layerName << std::endl;
        std::cout << "\tDescription: " << layerProperties.description << std::endl;
        std::cout << "\tImplementation Version: " << layerProperties.implementationVersion << std::endl;
        std::cout << "\tSpec Version: " << layerProperties.specVersion << std::endl;
        std::cout << "\t-----------------------------------" << std::endl;
    }

    // Check if given Validation Layer is in list of given Validation Layers
    for (const auto& validationLayer : validationLayers) {
        bool hasLayer = false;
        for (const auto& availableLayer : availableLayers) {
            if (strcmp(validationLayer, availableLayer.layerName) == 0) {
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

void VulkanRenderer::getPhysicalDevice() {
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
        if (this->checkDeviceSuitable(device)) {
            this->mainDevice.physicalDevice = device;
            break;
        }
    }
}

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device) {

    /*
    // Information abaout the device itself (ID, Name, Type Vendor, etc)
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // information about what the device can do (geo, Shader, tess, shader, wide lines, etc)
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    */

    QueueFamilyIndices indices = getQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainValid = false;
    if (extensionsSupported) {
        SwapChainDetails swapChainDetails = getSwapChainDetails(device);
        swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
    }

    return indices.isValid() && extensionsSupported && swapChainValid;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device) {

    QueueFamilyIndices indices;

    // Get all Queue Family Property info for the given device
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

    // Go through each queue family and check if it has at least 1 of the requered types of queue
    int i = 0;
    for (const auto& queueFamily : queueFamilyList) {

        // First check if queue has at least 1 queue in that family (could have no queue)
        // Queue cam be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if has
        // requered type
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i; // if queue family is valid then get index
        }

        // Check if Queue Family support presentation
        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport); // TODO: validar se result OK
        // check if queue is presentation type (can bo boyh graphics and presentation)
        if (queueFamily.queueCount > 0 && presentationSupport) {
            indices.presentationFamily = i;
        }

        // check if queue family indices are in valid state, stop searching if so
        if (indices.isValid()) {
            break;
        }

        i++;
    }

    return indices;
}

void VulkanRenderer::createLogicalDevices() {

    // Get the queue family indices for the chosen Physical device
    QueueFamilyIndices indices = this->getQueueFamilies(this->mainDevice.physicalDevice);

    // vector for queue creation information, and set for family indices
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = {indices.graphicsFamily, indices.presentationFamily};

    // Queues the logical device needs to create and info to do so
    for (int queueFamiyIndex : queueFamilyIndices) {

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamiyIndex; // The index of the family to create a from
        queueCreateInfo.queueCount = 1;                     // Numbers of queues to create
        float priority = 1.0f;
        queueCreateInfo.pQueuePriorities =
            &priority; // Vulkan needs to know how to handle multiple queues, so decide priority (1 is hight)

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Information to create logical device (sometimes called "device")
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // Number queueCreateInfos
    deviceCreateInfo.pQueueCreateInfos =
        queueCreateInfos.data(); // List of queueCreateInfos so device can create required queues

    deviceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());                 // Number of enable logical device extentions
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data(); // List of enable logical device extentions

    // Physical Device Features the Logical Device will be using
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures; // Physica device features logica device will use

    // Create the Logical device for the givem physical device
    VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a logical device");
    }

    // queues are created the same time as the device
    // so we want handle to queues
    // From given logical device, of given Queue Family, of given Queue Index(0 since only one), place reference in
    // given Vkqueue
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &this->graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &this->presentationQueue);
}

void VulkanRenderer::createSurface() {

    // Create Surface (creates a surface creste info struct, runs the create surface function, returns result)
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a surface!");
    }
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
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
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount,
                                                  swapChainDetails.presentationModes.data());
    }

    return swapChainDetails;
}
