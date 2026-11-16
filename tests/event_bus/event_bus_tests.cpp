#include "event_bus/EventBus.h"

#include "event_bus/SubscriptionHandle.h"

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <utility>
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

TEST_CASE("every subscriber receives the event, in subscription order", "[event_bus][publish]") {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle a = bus.subscribe<Ping>(record(log, "A"));
    SubscriptionHandle b = bus.subscribe<Ping>(record(log, "B"));
    SubscriptionHandle c = bus.subscribe<Ping>(record(log, "C"));

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"A", "B", "C"};

    CHECK(log == expected_log);
}

TEST_CASE("publish only reaches subscribers of that event type", "[event_bus][publish]") {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle ping = bus.subscribe<Ping>(record(log, "Ping"));
    SubscriptionHandle pong = bus.subscribe<Pong>(record(log, "Pong"));

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"Ping"};

    CHECK(log == expected_log);

    bus.publish(Pong{});

    expected_log = {"Ping", "Pong"};

    CHECK(log == expected_log);
}

TEST_CASE("publishing an event with no subscribers does nothing", "[event_bus][publish]") {
    std::vector<std::string> log;
    EventBus bus;

    CHECK_NOTHROW(bus.publish(Ping{42}));

    SubscriptionHandle ping = bus.subscribe<Ping>(record(log, "Ping"));

    CHECK_NOTHROW(bus.publish(Pong{}));

    CHECK(log.empty());
}

TEST_CASE("destroying a handle ends its subscription", "[event_bus][handle]") {
    std::vector<std::string> log;
    EventBus bus;

    std::vector<std::string> expected_log = {"Ping"};

    {
        SubscriptionHandle ping = bus.subscribe<Ping>(record(log, "Ping"));

        bus.publish(Ping{42});

        REQUIRE(log == expected_log);
    }

    bus.publish(Ping{42});

    CHECK(log == expected_log);
}

TEST_CASE("move-assigning a handle transfers the subscription", "[event_bus][handle]") {
    std::vector<std::string> log;
    EventBus bus;

    std::vector<std::string> expected_log = {"Original"};

    {
        SubscriptionHandle moved;

        {
            SubscriptionHandle original = bus.subscribe<Ping>(record(log, "Original"));

            moved = std::move(original);
        }

        bus.publish(Ping{42});

        REQUIRE(log == expected_log);
    }

    bus.publish(Ping{42});

    CHECK(log == expected_log);
}

TEST_CASE("move-constructing a handle transfers the subscription", "[event_bus][handle]") {
    std::vector<std::string> log;
    EventBus bus;

    std::vector<std::string> expected_log = {"Original"};

    std::optional<SubscriptionHandle> moved;

    {
        SubscriptionHandle original = bus.subscribe<Ping>(record(log, "Original"));

        moved.emplace(std::move(original));
    }

    bus.publish(Ping{42});

    REQUIRE(log == expected_log);

    moved.reset();

    bus.publish(Ping{42});

    CHECK(log == expected_log);
}
