#include "VulkanRenderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <string>

SDL_Window* window = nullptr;
VulkanRenderer vulkanRenderer;

bool initWindow(const std::string& sName = "Teste", const int width = 800, const int height = 600) {

    // 1. Initialize SDL3
    if (!SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11")) {
        std::cerr << "SDL X11 Failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // 2. Create Window with Vulkan support
    window = SDL_CreateWindow(sName.c_str(), width, height, SDL_WINDOW_VULKAN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

int main() {

    if (!initWindow("Teste", 800, 600))
        return SDL_APP_FAILURE;

    if (!vulkanRenderer.init(window))
        return SDL_APP_FAILURE;

    // --- Main Loop ---
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
        // Vulkan rendering would go here
    }

    vulkanRenderer.cleanup();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return SDL_APP_SUCCESS;
}
