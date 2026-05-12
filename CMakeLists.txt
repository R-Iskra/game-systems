cmake_minimum_required(VERSION 3.20)
project(GameSystems)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_state_machine
    core/state_machine/StateMachine.cpp
    tests/state_machine/main.cpp
)

target_include_directories(test_state_machine PRIVATE
    core/state_machine
    tests/state_machine
)

add_executable(test_event_bus
    core/event_bus/EventBus.cpp
    core/event_bus/SubscriptionHandle.cpp
    tests/event_bus/main.cpp
)

target_include_directories(test_event_bus PRIVATE
    core/event_bus
    tests/event_bus
)