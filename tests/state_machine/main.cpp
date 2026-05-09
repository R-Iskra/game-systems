#include <iostream>

#include "../../core/state_machine/StateMachine.h"
#include "TestStates.h"

void log_state(const StateMachine& sm) {
    std::cout << "Current state: " << sm.get_current_state()->get_name() << "\n";
}

int main() {
    EntityContext ctx;
    StateMachine sm;

    auto idle = std::make_unique<IdleState>();
    auto walk = std::make_unique<WalkState>();
    auto jump = std::make_unique<JumpState>();

    IdleState* idle_ptr = idle.get();
    WalkState* walk_ptr = walk.get();
    JumpState* jump_ptr = jump.get();

    idle_ptr->set_transitions(walk_ptr, jump_ptr);
    walk_ptr->set_transitions(idle_ptr, jump_ptr);
    jump_ptr->set_transitions(idle_ptr);

    sm.add_state(std::move(idle));
    sm.add_state(std::move(walk));
    sm.add_state(std::move(jump));

    sm.start("Idle", ctx);

    std::cout << "--- Initial ---\n";
    log_state(sm);

    std::cout << "--- Move Right ---\n";
    ctx.input_move_right = true;
    sm.update(ctx, 0.016f);
    log_state(sm);

    std::cout << "--- Jump ---\n";
    ctx.input_jump = true;
    sm.update(ctx, 0.016f);
    log_state(sm);

    std::cout << "--- Land ---\n";
    ctx.input_jump = false;
    ctx.is_on_floor = true;
    ctx.input_move_right = false;
    sm.update(ctx, 0.016f);
    log_state(sm);

    return 0;
}