#include "state_machine/StateMachine.h"

#include "state_machine/EntityContext.h"
#include "state_machine/IState.h"

#include <catch2/catch_test_macros.hpp>

#include <memory>

namespace {
    class NoTransitionState : public IState {
        public:
            void enter(EntityContext&) override {}
            void update(EntityContext&, float) override {}
            void exit(EntityContext&) override {}

            IState* get_transition(EntityContext&) override {
                return nullptr;
            }

            const char* get_name() const override { return "NoTransitionState"; }
    };
}   // namespace

TEST_CASE("starting enters the named state", "[state_machine][start]") {
    EntityContext ctx;
    StateMachine sm;

    sm.add_state(std::make_unique<NoTransitionState>());

    sm.start("NoTransitionState", ctx);

    CHECK(sm.is_in_state("NoTransitionState"));
}