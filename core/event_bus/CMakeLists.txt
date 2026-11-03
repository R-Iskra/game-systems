add_library(event_bus STATIC EventBus.cpp SubscriptionHandle.cpp)
target_include_directories(event_bus PUBLIC ${PROJECT_SOURCE_DIR}/core)