#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vector>

// Structure for Vertex Data
struct Vertex {
    float pos[3];
    float color[3];
};

// --- Helper Functions (Briefly) ---
// Note: In a real scenario, you need to create VkInstance, 
// VkSurfaceKHR, and Vulkan memory management (e.g., VMA).

void createLogicalDevice(VkPhysicalDevice physicalDevice, VkDevice& device, VkQueue& graphicsQueue, uint32_t& graphicsQueueFamily) {
    // 1. Find Queue Family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    
    graphicsQueueFamily = 0;
    for(int i=0; i<queueFamilyCount; i++){
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamily = i; break;
        }
    }

    // 2. Create Device Queue Info
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // 3. Create Logical Device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    
    // Required Extensions (Swapchain)
    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
}

// Minimal Memory Type Finder
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

// VBO/IBO Creation
void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, 
                  VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

int main() {
    // 1. Init SDL3
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SDL3 Vulkan Linux", 800, 600, SDL_WINDOW_VULKAN);
    
    // 2. Load Vulkan Instance via SDL
    uint32_t extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    // ... [Create VkInstance here] ...

    // 3. Physical Device Selection (Simple - first GPU)
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    // vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    // std::vector<VkPhysicalDevice> devices(deviceCount);
    // vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    // physicalDevice = devices[0]; 

    // 4. Logical Device
    VkDevice device;
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;
    // createLogicalDevice(physicalDevice, device, graphicsQueue, graphicsQueueFamily);

    // 5. Vertex Buffer (VBO)
    std::vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
    };
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    // createBuffer(..., VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ..., vertexBuffer, vertexBufferMemory);

    // 6. Index Buffer (IBO)
    std::vector<uint16_t> indices = {0, 1, 2};
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    // createBuffer(..., VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ..., indexBuffer, indexBufferMemory);

    // --- Main Loop ---
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) quit = true;
        }
    }

    // Cleanup
    // vkDestroyBuffer(device, indexBuffer, nullptr);
    // vkFreeMemory(device, indexBufferMemory, nullptr);
    // vkDestroyBuffer(device, vertexBuffer, nullptr);
    // vkFreeMemory(device, vertexBufferMemory, nullptr);
    // vkDestroyDevice(device, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
