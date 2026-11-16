#include "event_bus/EventBus.h"

#include "event_bus/SubscriptionHandle.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using namespace game_systems;

namespace {

struct Ping {
    int value;
};

struct Pong {};

auto record(std::vector<std::string>& log, std::string label) {
    return [&log, label](const auto&) { log.push_back(label); };
}

} // namespace
