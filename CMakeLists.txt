cmake_minimum_required(VERSION 3.20)
project(GameSystems)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wconversion -Wpedantic)

add_subdirectory(core/state_machine)
add_subdirectory(core/event_bus)

add_subdirectory(examples/state_machine)
add_subdirectory(examples/event_bus)