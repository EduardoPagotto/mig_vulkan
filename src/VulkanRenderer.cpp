#include "VulkanRenderer.hpp"
#include "Ultilities.hpp"
#include "VulkanValidation.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

int VulkanRenderer::init_vulkan() {
    try {
        this->createInstance();
        this->createDebugCallback();
        this->createSurface();     // create before physical
        this->getPhysicalDevice(); // now need see if support surface
        this->createLogicalDevices();
        this->createSwapChain();
        this->createGraphicsPipeline();
    } catch (const std::runtime_error& e) {
        printf("Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::cleanup() {

    for (auto image : this->swapchainImages) {
        vkDestroyImageView(this->mainDevice.logicalDevice, image.imageView, nullptr);
    }

    vkDestroySwapchainKHR(this->mainDevice.logicalDevice, this->swapchain, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyDevice(this->mainDevice.logicalDevice, nullptr);

    if (validationEnabled) {
        DestroyDebugReportCallbackEXT(this->instance, this->callback, nullptr);
    }

    vkDestroyInstance(this->instance, nullptr);
}

void VulkanRenderer::createInstance() {

    if (validationEnabled && !VulkanRenderer::checkValidationLayerSupport()) {
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
    if (validationEnabled) {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // check Instance Extentions suppoted..
    if (!VulkanRenderer::checkInstanceExtentionsSupport(&instanceExtensions)) {
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
    VkResult result = vkCreateInstance(&createInfo, nullptr, &this->instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
    }
}

void VulkanRenderer::createDebugCallback() {
    // Only create callback if validation enabled
    if (!validationEnabled) {
        return;
    }

    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                               VK_DEBUG_REPORT_WARNING_BIT_EXT; // Which validation reports should initiate callback
    callbackCreateInfo.pfnCallback = debugCallback;             // Pointer to callback function itself

    // Create debug callback with custom create function
    VkResult result = CreateDebugReportCallbackEXT(this->instance, &callbackCreateInfo, nullptr, &this->callback);
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
            if (strcmp(checkExtention, extention.extensionName) == 0) {
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

void VulkanRenderer::createSwapChain() {
    // Get Swap Chain details so we cam pick best setting
    SwapChainDetails swapchainDetails = this->getSwapChainDetails(this->mainDevice.physicalDevice);

    // Find optimal surface value for our swap chain
    VkSurfaceFormatKHR surrfaceFormat = VulkanRenderer::chooseBestSurfaceFormat(swapchainDetails.formats);
    VkPresentModeKHR presentMode = VulkanRenderer::chooseBestPresentationMode(swapchainDetails.presentationModes);
    VkExtent2D extent = this->chooseSwapExtent(swapchainDetails.surfaceCapabilities);

    // how many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
    uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;

    // If imagecount higher than max the clamp down to max
    // If 0, then limitless
    if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 &&
        swapchainDetails.surfaceCapabilities.maxImageCount < imageCount) {
        imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
    }

    // Create information for swap chain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = this->surface;                          // Swapchain surface
    swapchainCreateInfo.imageFormat = surrfaceFormat.format;              // Swapchain format
    swapchainCreateInfo.imageColorSpace = surrfaceFormat.colorSpace;      // Swapchain color space
    swapchainCreateInfo.presentMode = presentMode;                        // Swapchain presentation mode
    swapchainCreateInfo.imageExtent = extent;                             // Swapchain image extents
    swapchainCreateInfo.minImageCount = imageCount;                       // Minimum image in swapchain
    swapchainCreateInfo.imageArrayLayers = 1;                             // Number of layers for each image in chain
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // What attachement image will be used as
    swapchainCreateInfo.preTransform =
        swapchainDetails.surfaceCapabilities.currentTransform; // Transform to perform on swap chain images
    swapchainCreateInfo.compositeAlpha =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // How to handle blending images with external graphics(e.g. other windows)
    swapchainCreateInfo.clipped =
        VK_TRUE; // Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)

    // Get Queue Family indices
    QueueFamilyIndices indices = this->getQueueFamilies(this->mainDevice.physicalDevice);

    // If Graphics and Presentation families are diferent, the swapchain must let images ge shared between families
    if (indices.graphicsFamily != indices.presentationFamily) {

        // Queue to share between
        uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentationFamily};

        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Image share handling
        swapchainCreateInfo.queueFamilyIndexCount = 2;                     // Number of queues to share images between
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;      // Array of queues to share between
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    // If old swap chain been destroyed and this one replaces it, then link old one to quickly hand over
    // responsabilities
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create Swapchain
    VkResult result =
        vkCreateSwapchainKHR(this->mainDevice.logicalDevice, &swapchainCreateInfo, nullptr, &this->swapchain);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Swapchain");
    }

    // Store for late reference
    this->swapchainImageFormat = surrfaceFormat.format;
    this->swapchainExtent = extent;

    // Get swap chain images (first count the values)
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(this->mainDevice.logicalDevice, this->swapchain, &swapChainImageCount, nullptr);

    std::vector<VkImage> images(swapChainImageCount);
    vkGetSwapchainImagesKHR(this->mainDevice.logicalDevice, this->swapchain, &swapChainImageCount, images.data());

    for (VkImage image : images) {
        // Store image handle
        SwapchainImage swapChainImage = {};
        swapChainImage.image = image;
        swapChainImage.imageView = this->createImageView(image, this->swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

        // Add to swapchain image list
        this->swapchainImages.push_back(swapChainImage);
    }
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const {
    //
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;                                // Image to create view for
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;             // Type of image (1D, 2D, 3D, Cube, etc)
    viewCreateInfo.format = format;                              // Format of image data
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Allows remapping of rgba component to other values
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // Subresources allow the view to view only a part of a image
    viewCreateInfo.subresourceRange.aspectMask =
        aspectFlags;                                    // which aspect of image to view (e.g. COLOR_BIT for view color)
    viewCreateInfo.subresourceRange.baseMipLevel = 0;   // Start mipmap level to start from
    viewCreateInfo.subresourceRange.levelCount = 1;     // Number of mipmap levels to view
    viewCreateInfo.subresourceRange.baseArrayLayer = 0; // Start array level to view from
    viewCreateInfo.subresourceRange.layerCount = 1;     // Numbers of array levels to view

    // Create image view and return it
    VkImageView imageView;
    VkResult result = vkCreateImageView(this->mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create an Image View!");
    }

    return imageView;
}

// Best format is subjective, but ours will be:
// Format     : VK_FORMAT_R8G8B8A8_UNFORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
// colorSpace : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {

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

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentationModes) {
    // Look for Mailbox presentation mode
    for (const auto& presentationMode : presentationModes) {
        if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentationMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR; // allways avaible by vulkan
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {

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
    newExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
                               std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));

    newExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
                                std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

    return newExtent;
}

void VulkanRenderer::getPhysicalDevice() {
    // Enumerate Physical devices the vkInstance can access
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

    // if no devices avaible, then none suport Vulkan!
    if (deviceCount == 0) {
        throw std::runtime_error("Cant find GPUs that support Vulkan Instance");
    }

    // get List of Physical devices
    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, deviceList.data());

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

    QueueFamilyIndices indices = this->getQueueFamilies(device);

    bool extensionsSupported = VulkanRenderer::checkDeviceExtensionSupport(device);

    bool swapChainValid = false;
    if (extensionsSupported) {
        SwapChainDetails swapChainDetails = this->getSwapChainDetails(device);
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, this->surface,
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
        float priority = 1.0F;
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

#ifdef SET_GLFW_ENABLE
    VkResult result = glfwCreateWindowSurface(this->instance, window, nullptr, &this->surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a surface!");
    }
#else
    bool result = SDL_Vulkan_CreateSurface(window, this->instance, nullptr, &this->surface);
    if (!result) {
        throw std::runtime_error("Failed to create a surface!");
    }
#endif
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
    SwapChainDetails swapChainDetails;

    // -- CAPABILITIES --
    // Get the surface capabilities for the given surface on the given physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &swapChainDetails.surfaceCapabilities);

    // -- FORMATS --
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);

    // If formats returned, get list of formats
    if (formatCount != 0) {
        swapChainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, swapChainDetails.formats.data());
    }

    // -- PRESENTATION MODES --
    uint32_t presentationCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentationCount, nullptr);

    // If presentation modes returned, get list of presentation modes
    if (presentationCount != 0) {
        swapChainDetails.presentationModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentationCount,
                                                  swapChainDetails.presentationModes.data());
    }

    return swapChainDetails;
}

void VulkanRenderer::createGraphicsPipeline() {

    // Read in SPIR-V code shaders
    auto vertexShadercode = readFile("./bin/vert.spv");
    auto fragmentShadercode = readFile("./bin/frag.spv");

    // Create shader modules
    VkShaderModule vertexShaderModule = this->createShaderModule(vertexShadercode);
    VkShaderModule fragmentShaderModule = this->createShaderModule(fragmentShadercode);

    // -- SHADER STAGE CREATION INFORMATION --
    // Vertex Stage creation information
    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // Shader stage name
    vertexShaderCreateInfo.module = vertexShaderModule;        // Shader module to be used by stage
    vertexShaderCreateInfo.pName = "main";                     // Entry point in to shader

    // Fragment Stage creation information
    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // Shader stage name
    fragmentShaderCreateInfo.module = fragmentShaderModule;        // Shader module to be used by stage
    fragmentShaderCreateInfo.pName = "main";                       // Entry point in to shader

    // Put shader creation info in to array
    // Graphics Pipeline creation info required array of shader stage creates
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderCreateInfo, fragmentShaderCreateInfo};

    // VkGraphicsPipelineCreateInfo
    // CREATE PIPELINE

    // Destroy Shader Module, no longer need after Pipeline create
    vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) const {
    //
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size(); // size of code
    shaderModuleCreateInfo.pCode =
        reinterpret_cast<const uint32_t*>(code.data()); // pointer to code(of uint32_t pointer type)

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a shader module");
    }

    return shaderModule;
}
