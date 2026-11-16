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

TEST_CASE("publish delivers the event's data to a subscriber", "[event_bus][publish]") {
    std::vector<int> received;
    EventBus bus;

    auto sub = bus.subscribe<Ping>([&received](const Ping& e) { received.push_back(e.value); });

    bus.publish(Ping{42});

    std::vector<int> expected_received = {42};

    CHECK(received == expected_received);
}
