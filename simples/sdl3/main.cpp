#include "VulkanRenderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <iostream>
#include <string>

SDL_Window* window = nullptr;
VulkanRenderer vulkanRenderer;

bool initWindow(const std::string& sName = "Teste", const int width = 800, const int height = 600) {

    // 1. Initialize SDL3
    if (!SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11")) {
        std::cerr << "SDL X11 Failed: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Failed: " << SDL_GetError() << '\n';
        return false;
    }

    // 2. Create Window with Vulkan support
    window = SDL_CreateWindow(sName.c_str(), width, height, SDL_WINDOW_VULKAN);
    if (window == nullptr) {
        std::cerr << "Window creation failed: " << SDL_GetError() << '\n';
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Vulkan SD3 Window Created");

    return true;
}

int main() {

    if (!initWindow("Teste")) {
        return SDL_APP_FAILURE;
    }

    if (vulkanRenderer.init(window) == EXIT_FAILURE) {
        return SDL_APP_FAILURE;
    }

    for (bool running = true; running;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Vulkan rendering would go here
        vulkanRenderer.draw();
    }

    vulkanRenderer.cleanup();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return SDL_APP_SUCCESS;
}
