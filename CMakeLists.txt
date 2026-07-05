cmake_minimum_required(VERSION 4.3.0)
project(VulkanTest)

set(CMAKE_CXX_FLAGS "-std=c++20 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs -fstandalone-debug")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# # inclui projetos externos
# include(FetchContent)

# # Configura o fastgltf e sua dependência obrigatória (simdjson)
# FetchContent_Declare(
#   fastgltf
#   GIT_REPOSITORY https://github.com/spnda/fastgltf
#   GIT_TAG        main  # Verifique a release mais recente no GitHub
# )
# FetchContent_MakeAvailable(fastgltf)

# # Configura o entt
# FetchContent_Declare(
#   EnTT
#   GIT_REPOSITORY https://github.com/skypjack/entt
#   GIT_TAG        main # Substitua pela versão desejada
# )
# FetchContent_MakeAvailable(EnTT)

#add_subdirectory(src/cevk)
#add_subdirectory(apps/sdl3)
add_subdirectory(apps/loader)


# clean
add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
                             COMMAND rm -rf ${CMAKE_BINARY_DIR}/.cmake
                             COMMAND rm -rf ${CMAKE_BINARY_DIR}/.ninja_*
                             COMMAND rm -rf ${CMAKE_SOURCE_DIR}/bin/*)
