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

TEST_CASE("assigning over an active handle ends the old subscription", "[event_bus][handle]") {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle sub = bus.subscribe<Ping>(record(log, "Old"));

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"Old"};

    REQUIRE(log == expected_log);

    sub = bus.subscribe<Ping>(record(log, "New"));

    bus.publish(Ping{42});

    expected_log = {"Old", "New"};

    CHECK(log == expected_log);
}

TEST_CASE(
    "subscribing during a publish takes effect after the publish", "[event_bus][reentrancy]"
) {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle late;
    bool subscribed = false;

    SubscriptionHandle first = bus.subscribe<Ping>([&](const Ping&) {
        log.push_back("First");

        if (!subscribed) {
            late = bus.subscribe<Ping>(record(log, "Late"));
            subscribed = true;
        }
    });

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"First"};

    REQUIRE(log == expected_log);

    bus.publish(Ping{42});

    expected_log = {"First", "First", "Late"};

    CHECK(log == expected_log);
}

TEST_CASE(
    "subscribing during a nested publish waits for the outermost publish", "[event_bus][reentrancy]"
) {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle late;
    bool ping_published = false;
    bool late_subscribed = false;

    SubscriptionHandle pong = bus.subscribe<Pong>([&](const Pong&) {
        log.push_back("Pong");

        if (!late_subscribed) {
            late_subscribed = true;

            late = bus.subscribe<Ping>(record(log, "Late"));
        }
    });

    SubscriptionHandle ping = bus.subscribe<Ping>([&](const Ping&) {
        log.push_back("Ping");

        if (!ping_published) {
            ping_published = true;

            bus.publish(Pong{});

            bus.publish(Ping{42});
        }
    });

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"Ping", "Pong", "Ping"};

    REQUIRE(log == expected_log);

    bus.publish(Ping{42});

    expected_log = {"Ping", "Pong", "Ping", "Ping", "Late"};

    CHECK(log == expected_log);
}

TEST_CASE(
    "unsubscribing during a publish stops later callbacks in the same publish",
    "[event_bus][reentrancy]"
) {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle second;

    SubscriptionHandle first = bus.subscribe<Ping>([&](const Ping&) {
        log.push_back("First");

        second = SubscriptionHandle();
    });

    second = bus.subscribe<Ping>(record(log, "Second"));

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"First"};

    REQUIRE(log == expected_log);

    bus.publish(Ping{42});

    expected_log = {"First", "First"};

    CHECK(log == expected_log);
}

TEST_CASE("unsubscribing yourself during a publish", "[event_bus][reentrancy]") {
    std::vector<std::string> log;
    EventBus bus;

    SubscriptionHandle self;

    self = bus.subscribe<Ping>([&](const Ping&) {
        log.push_back("Self");

        self = SubscriptionHandle();
    });

    SubscriptionHandle after = bus.subscribe<Ping>(record(log, "After"));

    bus.publish(Ping{42});

    std::vector<std::string> expected_log = {"Self", "After"};

    REQUIRE(log == expected_log);

    bus.publish(Ping{42});

    expected_log = {"Self", "After", "After"};

    CHECK(log == expected_log);
}
