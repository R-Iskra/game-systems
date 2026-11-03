cmake_minimum_required(VERSION 3.20)
project(GameSystems)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(core/state_machine)
add_subdirectory(core/event_bus)

add_subdirectory(examples/state_machine)
add_subdirectory(examples/event_bus)