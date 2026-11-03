add_library(state_machine STATIC StateMachine.cpp)
target_include_directories(state_machine PUBLIC ${PROJECT_SOURCE_DIR}/core)
target_link_libraries(state_machine PRIVATE project_warnings)