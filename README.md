# Vulkan API
Migrating my knowledge from OpenGL to Vulkan

## Features of project:
- Editor: VSCode (clangd, Clang-Format, CMake Tools, CodeLLDB)
- Build tool: CMAKE
- Compiler: Clang (mostly C++20)
- Debug: lldb
- Ident: clang-format (file: .clang-format)
- LIBS an API's:
  - Vulkan 1.1
  - SDL3, SDL3_Image, SDL3_TTF
  - GLFW3

## Deps develop
```bash
# wayland-devel
sudo dnf install wayland-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel

# dev
sudo dnf group install c-development
sudo dnf group install development-tools

# Develop c++ clang / llvm / lldb
sudo dnf install clang clang-tools-extra
sudo dnf install cmake cmake-data cmake-rpm-macros jsoncpp libstdc++-static llvm-static llvm-devel llvm-test autoconf automake
sudo dnf install lldb lldb-devel lld-devel lld-libs.x86_64
sudo dnf install compiler-rt

# change linker to clang
alternatives --config ld
#change to -> 2           /usr/bin/ld.lld

sudo dnf install mesa-dri-drivers mesa-libGL freeglut-devel glm-devel glew glew-devel libGLEW
sudo dnf install bullet bullet-devel bullet-extras bullet-extras-devel

# SDL3
sudo dnf install SDL3-static SDL3-devel SDL3_image-devel SDL3_ttf SDL3_ttf-devel

# Lib vulkan
sudo dnf install vulkan-loader-devel vulkan-tools vulkan-utility-libraries-devel glslang glslc VulkanMemoryAllocator-devel

# Teste do GLFW
sudo dnf install glfw-devel
```

## Build and test:
```bash

cmake -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE \
      -DCMAKE_TOOLCHAIN_FILE=./toolchain/clang.cmake \
      -B build

make -C build -j 4

# test SDL3
# wayland still has erros, force X11
SDL_VIDEODRIVER=x11 ./bin/sdl_test

# test with GLFW
./bin/glwf_test
```

# Como funciona a API!

## Sequencia de Inicialização do VulKan:

1- Inicializa SDL_VIDEO

2- Criar window

3- Validar extençoes do SDL com Vulkan
  3.1- executar SDL_Vulkan_GetInstanceExtensions para pegar totais de extencoes SDL
  3.2- executar vkEnumerateInstanceExtensionProperties para pegar lista de extencoes Vulkan
  3.3- Percorre (3.1) e valida exitencia em cada (3.2)

4- Criar Instancia
  4.1- criar struct VkApplicationInfo
  4.2- Criar struct VkInstanceCreateInfo com (3.1) e (4.1)

5- Pegar o PhysicalDevice
  5.1 - executar vkEnumeratePhysicalDevices para pegar lista de PhysicalDevice
  5.2 - executar vkGetPhysicalDeviceQueueFamilyProperties cada item da lista de 5.1
  5.3 - criar QueueFamilyIndices e validar se tem flags compativeis com cada 5.2
  5.4 - usar o primeiro device no struct mainDevice.physicalDevice

6- Criar o LogicalDevice
  6.1 - pegar indices com struct QueueFamilyIndices com parametro (5.4)
  6.2 - criar struct VkDeviceQueueCreateInfo usando indice de (6.1)
  6.3 - criar struct VkDeviceCreateInfo usando (6.2)
  6.4 - criar struct VkPhysicalDeviceFeatures
  6.5 - usar 6.4 em 6.3
  6.6 - executar vkCreateDevice passando 5.4, 6.2, resultado em mainDevice.logicalDevice
  6.7 - executar vkGetDeviceQueue pasando 5.3 e 6.6 resultado em VkQueue

## Swapchain

7-
