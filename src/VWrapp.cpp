#include "VWrapp.hpp"
#include "VWrappUtils.hpp"
#include <cstring>
#include <set>
#include <stdexcept>

namespace ce {
    VWrapp::~VWrapp() {
        // cleanup
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(logicalDevice, nullptr);

        if (validationEnabled) {
            DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        }

        vkDestroyInstance(instance, nullptr);
    }

    void VWrapp::init_device() {
        createInstance();
        createDebugCallback();
        createSurface();        // create before physical
        getNewPhysicalDevice(); // now need see if support surface
        createLogicalDevice();
    }

    void VWrapp::createInstance() {

        if (validationEnabled && !CheckValidationLayerSupport()) {
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
        if (!CheckInstanceExtensionSupport(&instanceExtensions)) {
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

    void VWrapp::createDebugCallback() {
        // Only create callback if validation enabled
        if (!validationEnabled) {
            return;
        }

        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
        callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        callbackCreateInfo.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT; // Which validation reports should initiate callback
        callbackCreateInfo.pfnCallback = DebugCallback;                      // Pointer to callback function itself

        // Create debug callback with custom create function
        VkResult result = CreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Debug Callback!");
        }
    }

    void VWrapp::createSurface() {

        // Create Surface (creates a surface creste info struct, runs the create surface function, returns result)
#ifdef SET_GLFW_ENABLE
        VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
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

    void VWrapp::getNewPhysicalDevice() {
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
                physicalDevice = device;
                break;
            }
        }

        // Get properties of our new device
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        // minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
    }

    void VWrapp::createLogicalDevice() {

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

    VkExtent2D VWrapp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {

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
} // namespace ce
