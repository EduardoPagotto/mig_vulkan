// #define VMA_IMPLEMENTATION // Only define in one .cpp file
// #include <vk_mem_alloc.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <iostream>

#define VK_USE_PLATFORM_XCB_KHR


// VkDevice ttt(SDL_GPUDevice* your_gpu_device) {
//     // 1. Obtenha o ID das propriedades do seu dispositivo GPU
//     SDL_PropertiesID props = SDL_GetGPUDeviceProperties(your_gpu_device);

//     // 2. Busque os handles usando as strings de propriedade do SDL3
//     VkInstance instance = (VkInstance)SDL_GetPointerProperty(props, SDL_PROP_GPU_DEVICE_VULKAN_INSTANCE_POINTER, NULL);
//     VkPhysicalDevice physicalDevice = (VkPhysicalDevice)SDL_GetPointerProperty(props, SDL_PROP_GPU_DEVICE_VULKAN_PHYSICAL_DEVICE_POINTER, NULL);
//     VkDevice device = (VkDevice)SDL_GetPointerProperty(props, SDL_PROP_GPU_DEVICE_VULKAN_DEVICE_POINTER, NULL);
//     return device;
// }

SDL_GPUDevice* ffff() {
     // Create a property group for the GPU device
    SDL_PropertiesID props = SDL_CreateProperties();

    // 1. Force the Vulkan driver (optional, defaults to best available)
    SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");

    // 2. Require hardware acceleration (skip software renderers like Lavapipe)
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_REQUIRE_HARDWARE_ACCELERATION_BOOLEAN, true);

    // 3. Enable specific Vulkan features
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_ANISOTROPY_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_INDIRECT_DRAW_FIRST_INSTANCE_BOOLEAN, true);

    // Create the device using the properties
    SDL_GPUDevice* device = SDL_CreateGPUDeviceWithProperties(props);

    // Properties are no longer needed after device creation
    SDL_DestroyProperties(props);

    if (device) {
        SDL_Log("Vulkan GPU Device created successfully!");
        SDL_DestroyGPUDevice(device);
    } else {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
    }

    return device;
    //SDL_Quit();
    //return 0;
}


VkPhysicalDevice get_physical_device(VkInstance instance) {

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        // Nenhuma GPU com suporte a Vulkan encontrada
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    for (const auto& device : devices) {
        // 1. Checar propriedades (Nome, tipo de GPU)
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // 2. Checar suporte a filas (Graphics Queue)
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        bool supportsGraphics = false;
        // Example: Checking each queue family for presentation support
        uint32_t presentFamilyIndex = 0;
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (SDL_Vulkan_GetPresentationSupport(instance, physicalDevice, i)) {
                // This queue family supports presentation
                presentFamilyIndex = i;
                supportsGraphics = true;
                break;
            }
        }


        // 3. Escolher a GPU (Exemplo: Preferir GPU dedicada)
        if (supportsGraphics) {
            physicalDevice = device;
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                return physicalDevice;
                //break; // Encontrou uma placa dedicada, pode parar a busca
            }
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        // Nenhuma GPU adequada encontrada
    }

    return nullptr;
}


int main(int argc, char *argv[]) {
    // 1. Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        std::cerr << "SDL_Init Failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 2. Create Window with Vulkan support
    SDL_Window* window = SDL_CreateWindow("SDL3 Vulkan Linux", 800, 600, SDL_WINDOW_VULKAN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    // 3. Get required Instance Extensions
    uint32_t sdl_extension_count = 0;
    const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
    if (!sdl_extensions) {
        std::cerr << "Failed to get extensions: " << SDL_GetError() << std::endl;
        return -1;   
    }

    // Convert to a vector for easier management
    std::vector<const char*> extensions(sdl_extensions, sdl_extensions + sdl_extension_count);

    // 4. Set up VkInstanceCreateInfo
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "SDL3 App";
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    // 4.1. Create the Instance
    VkInstance instance;
    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }



    // 5. Create the Vulkan Surface
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        std::cerr << "Failed to create Vulkan surface: " << SDL_GetError() << std::endl;
        return -1;
    }







   // --- Main Loop ---
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        // Vulkan rendering would go here
    }


    // Cleanup
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
