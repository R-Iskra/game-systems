#include "state_machine/StateMachine.h"

#include "state_machine/EntityContext.h"
#include "state_machine/IState.h"

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace game_systems;

namespace {

struct HookLog {
    std::vector<std::string> calls;
    std::vector<float> deltas;
};

class TestState : public IState {
  public:
    TestState(std::string name, HookLog& log) : name(std::move(name)), log(log) {}
    ~TestState() override { log.calls.push_back(name + ".destroyed"); }

    void enter(EntityContext&) override {
        log.calls.push_back(name + ".enter");
        if (on_enter) on_enter();
    }
    void update(EntityContext&, float delta) override {
        log.calls.push_back(name + ".update");
        log.deltas.push_back(delta);
    }
    void exit(EntityContext&) override {
        log.calls.push_back(name + ".exit");
        if (on_exit) on_exit();
    }

    IState* get_transition(const EntityContext&) const override {
        log.calls.push_back(name + ".check");
        return target;
    }
    const char* get_name() const override { return name.c_str(); }

    IState* target = nullptr;
    std::function<void()> on_enter;
    std::function<void()> on_exit;

  private:
    std::string name;
    HookLog& log;
};

TestState* add_test_state(StateMachine& sm, std::string name, HookLog& log) {
    auto state = std::make_unique<TestState>(std::move(name), log);
    TestState* raw = state.get();
    sm.add_state(std::move(state));
    return raw;
}

} // namespace

TEST_CASE("start finds any added state by its name", "[state_machine][add_state][start]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    std::vector<std::string> expected_calls = {"B.enter"};

    CHECK(log.calls == expected_calls);
}

TEST_CASE("start makes the named state current", "[state_machine][start]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    CHECK(sm.get_current_state() == b);
}

TEST_CASE(
    "start with an unknown name throws and leaves the machine unstarted", "[state_machine][start]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.start("Missing", ctx), std::invalid_argument);

    CHECK(sm.get_current_state() == nullptr);
    CHECK(log.calls.empty());

    CHECK_NOTHROW(sm.start("A", ctx));
}

TEST_CASE("start on a started machine throws and leaves it unchanged", "[state_machine][start]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_THROWS_AS(sm.start("C", ctx), std::logic_error);

    std::vector<std::string> expected_calls = {"B.enter"};

    CHECK(sm.get_current_state() == b);
    CHECK(log.calls == expected_calls);
}

TEST_CASE(
    "start with a null name throws and leaves the machine unstarted", "[state_machine][start]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.start(nullptr, ctx), std::invalid_argument);

    CHECK(sm.get_current_state() == nullptr);
    CHECK(log.calls.empty());

    CHECK_NOTHROW(sm.start("A", ctx));
}

TEST_CASE(
    "add_state with a duplicate name throws and keeps the original state",
    "[state_machine][add_state]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.add_state(std::make_unique<TestState>("B", log)), std::invalid_argument);

    sm.start("B", ctx);

    CHECK(sm.get_current_state() == b);
}

TEST_CASE(
    "add_state with a null state throws and leaves the machine unchanged",
    "[state_machine][add_state]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.add_state(nullptr), std::invalid_argument);

    CHECK(log.calls.empty());

    sm.start("B", ctx);

    CHECK(sm.get_current_state() == b);
}

TEST_CASE(
    "add_state on a started machine throws and leaves it unchanged", "[state_machine][add_state]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_THROWS_AS(sm.add_state(std::make_unique<TestState>("D", log)), std::logic_error);

    CHECK(sm.get_current_state() == b);

    std::vector<std::string> expected_calls = {"B.enter", "D.destroyed"};

    CHECK(log.calls == expected_calls);

    CHECK_THROWS(sm.force_transition("D", ctx));
}

TEST_CASE(
    "update without a transition checks then updates the current state", "[state_machine][update]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    sm.update(ctx, 0.25f);

    std::vector<std::string> expected_calls = {"B.enter", "B.check", "B.update"};

    CHECK(log.calls == expected_calls);

    std::vector<float> expected_deltas = {0.25f};

    CHECK(log.deltas == expected_deltas);
}

TEST_CASE("update with a transition makes the target current", "[state_machine][update]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    TestState* c = add_test_state(sm, "C", log);

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    b->target = c;

    sm.update(ctx, 0.25f);

    CHECK(sm.get_current_state() == c);
}

TEST_CASE(
    "update with a transition exits the current state then enters and updates the target",
    "[state_machine][update]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    TestState* c = add_test_state(sm, "C", log);

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    b->target = c;

    sm.update(ctx, 0.25f);

    std::vector<std::string> expected_calls = {
        "B.enter", "B.check", "B.exit", "C.enter", "C.update"
    };

    CHECK(log.calls == expected_calls);

    std::vector<float> expected_deltas = {0.25f};

    CHECK(log.deltas == expected_deltas);
}

TEST_CASE(
    "update on an unstarted machine throws and leaves it unstarted", "[state_machine][update]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.update(ctx, 0.25f), std::logic_error);

    CHECK(sm.get_current_state() == nullptr);

    CHECK(log.calls.empty());

    CHECK_NOTHROW(sm.start("B", ctx));
}

TEST_CASE(
    "update with a transition to an unowned state throws and leaves the machine unchanged",
    "[state_machine][update]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    TestState d("D", log);

    b->target = &d;

    CHECK_THROWS_AS(sm.update(ctx, 0.25f), std::logic_error);

    CHECK(sm.get_current_state() == b);

    std::vector<std::string> expected_calls = {"B.enter", "B.check"};

    CHECK(log.calls == expected_calls);
}

TEST_CASE(
    "update rejects an unowned state that shares an owned state's name", "[state_machine][update]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    TestState imposter("C", log);

    b->target = &imposter;

    CHECK_THROWS_AS(sm.update(ctx, 0.25f), std::logic_error);

    CHECK(sm.get_current_state() == b);

    std::vector<std::string> expected_calls = {"B.enter", "B.check"};

    CHECK(log.calls == expected_calls);
}

TEST_CASE(
    "force_transition exits the current state then enters the named state",
    "[state_machine][force_transition]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    sm.force_transition("C", ctx);

    std::vector<std::string> expected_calls = {"B.enter", "B.exit", "C.enter"};

    CHECK(log.calls == expected_calls);
}

TEST_CASE("force_transition makes the named state current", "[state_machine][force_transition]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    TestState* c = add_test_state(sm, "C", log);

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    sm.force_transition("C", ctx);

    CHECK(sm.get_current_state() == c);
}

TEST_CASE(
    "force_transition on an unstarted machine throws and leaves it unstarted",
    "[state_machine][force_transition]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.force_transition("B", ctx), std::logic_error);

    CHECK(sm.get_current_state() == nullptr);

    CHECK(log.calls.empty());

    CHECK_NOTHROW(sm.start("B", ctx));
}

TEST_CASE(
    "force_transition with an unknown name throws and leaves the machine unchanged",
    "[state_machine][force_transition]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_THROWS_AS(sm.force_transition("Missing", ctx), std::invalid_argument);

    CHECK(sm.get_current_state() == b);

    std::vector<std::string> expected_calls = {"B.enter"};

    CHECK(log.calls == expected_calls);
}

TEST_CASE(
    "force_transition with a null name throws and leaves the machine unchanged",
    "[state_machine][force_transition]"
) {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_THROWS_AS(sm.force_transition(nullptr, ctx), std::invalid_argument);

    CHECK(sm.get_current_state() == b);

    std::vector<std::string> expected_calls = {"B.enter"};

    CHECK(log.calls == expected_calls);
}

TEST_CASE("is_in_state is true for the current state's name", "[state_machine][is_in_state]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK(sm.is_in_state("B"));
}

TEST_CASE("is_in_state is false for another state in the machine", "[state_machine][is_in_state]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_FALSE(sm.is_in_state("A"));
}

TEST_CASE("is_in_state is false on an unstarted machine", "[state_machine][is_in_state]") {
    HookLog log;
    StateMachine sm;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_FALSE(sm.is_in_state("B"));
}

TEST_CASE("is_in_state with an unknown name throws", "[state_machine][is_in_state]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_THROWS_AS(sm.is_in_state("Missing"), std::invalid_argument);
}

TEST_CASE(
    "is_in_state with an unknown name throws on an unstarted machine",
    "[state_machine][is_in_state]"
) {
    HookLog log;
    StateMachine sm;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.is_in_state("Missing"), std::invalid_argument);
}

TEST_CASE("is_in_state with a null name throws", "[state_machine][is_in_state]") {
    HookLog log;
    StateMachine sm;
    EntityContext ctx;

    sm.add_state(std::make_unique<TestState>("A", log));
    TestState* b = add_test_state(sm, "B", log);
    sm.add_state(std::make_unique<TestState>("C", log));

    sm.start("B", ctx);

    REQUIRE(sm.get_current_state() == b);

    CHECK_THROWS_AS(sm.is_in_state(nullptr), std::invalid_argument);
}

TEST_CASE(
    "is_in_state with a null name throws on an unstarted machine", "[state_machine][is_in_state]"
) {
    HookLog log;
    StateMachine sm;

    sm.add_state(std::make_unique<TestState>("A", log));
    sm.add_state(std::make_unique<TestState>("B", log));
    sm.add_state(std::make_unique<TestState>("C", log));

    CHECK_THROWS_AS(sm.is_in_state(nullptr), std::invalid_argument);
}
