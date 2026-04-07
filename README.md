# Test Vulkan

## Dependencias para DEV
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

## Build engine and examples and test:
```bash

cmake -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE \
      -DCMAKE_TOOLCHAIN_FILE=./toolchain/clang.cmake \
      -B build

make -C build -j 4

# wayland still has erros, force X11
SDL_VIDEODRIVER=x11 ./bin/sdl_test

# test with GLFW
./bin/glwf_test
```

main1.cpp te inicializacao com o VMA mas preciso buscar os valores de algun jeito
