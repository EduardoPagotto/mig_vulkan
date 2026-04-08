#include "VulkanRenderer.hpp"
#include "Ultilities.hpp"
#include <SDL3/SDL_vulkan.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

VulkanRenderer::VulkanRenderer() {}

VulkanRenderer::~VulkanRenderer() {}

int VulkanRenderer::init(GLFWwindow* window) {
    this->window = window;

    try {
        this->createInstance();
        this->getPhysicalDevice();
        this->createLogicalDevices();
    } catch (const std::runtime_error& e) {
        printf("Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}

void VulkanRenderer::createInstance() {

    // Information about the aplication itself
    // Most data here doesn't affect program and is for developer convinience
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan app Teste";         // Custom name of the aplication
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version app
    appInfo.pEngineName = "No engine";                     // Engine name
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      // engine version
    appInfo.apiVersion = VK_API_VERSION_1_1;               // the version of vulkan

    // Create information for a VkInstance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // createInfo.pNext = nullptr;
    // createInfo.flags = VK_WHATEVER | WHAT_EVER2
    createInfo.pApplicationInfo = &appInfo;

    // Create a List to hold instance extencios
    std::vector<const char*> instanceExtentions = std::vector<const char*>();

    // set up extentions will use
    uint32_t glfwExtentionCount = 0; // GLFW may require multiple extentions
    const char** glfwExtentions;     // Extentions passed as array of cstring,
    ;                                // so need pointer (the array) to pointer(the string)
    // Get glfw extentions
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    // Add glwf extentions to list of extentions
    for (size_t i = 0; i < glfwExtentionCount; i++) {
        instanceExtentions.push_back(glfwExtentions[i]);
    }

    // check Instance Extentions suppoted..
    if (!this->checkInstanceExtentionsSupport(&instanceExtentions)) {
        throw std::runtime_error("vkInstance does no suport requerid extentions!");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtentions.size());
    createInfo.ppEnabledExtensionNames = instanceExtentions.data();

    // TODO: Set a validation layer tha instace will use
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    // Create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
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

void VulkanRenderer::cleanup() {
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
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

    QueueFamilyIndices indices = this->getQueueFamillies(device);

    return indices.isValid();
}

QueueFamilyIndices VulkanRenderer::getQueueFamillies(VkPhysicalDevice device) {

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
    QueueFamilyIndices indices = this->getQueueFamillies(this->mainDevice.physicalDevice);

    // Queues the logical device needs to create and info to do so (only 1 for now, will add more late)
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily; // The index of the family to create a from
    queueCreateInfo.queueCount = 1;                            // Numbers of queues to create
    float priority = 1.0f;
    queueCreateInfo.pQueuePriorities =
        &priority; // Vulkan needs to know how to handle multiple queues, so decide priority (1 is hight)

    // Information to create logical device (sometimes called "device")
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1; // Number of queue create infos
    deviceCreateInfo.pQueueCreateInfos =
        &queueCreateInfo;                       // List of queue create infos so device can create required queues
    deviceCreateInfo.enabledExtensionCount = 0; // Number of enable logical device extentions
    deviceCreateInfo.ppEnabledExtensionNames = nullptr; // List of enable logicva device extentions

    // Physical Device Features the Logical Device will be using
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures; // Physica device features logica device will use

    // Create the Logical device for the givem physical device
    VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a logical device");
    }

    // queues are createat the same time as the device
    // so we want handle to queues
    // From given logical device, of given Queue Family, of given Queue Index(0 since only one), place reference in
    // given Vkqueue
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &this->graphicsQueue);
}
