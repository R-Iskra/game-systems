#include "StateMachine.h"

#include <stdexcept>

namespace game_systems {

void StateMachine::add_state(std::unique_ptr<IState> state) {
    if (state == nullptr) {
        throw std::invalid_argument("State is null");
    }

    std::string name = state->get_name();
    states[name] = std::move(state);
}

void StateMachine::start(const char* initial_state_name, EntityContext& ctx) {
    IState* state = require_state(initial_state_name);
    current_state = state;
    current_state->enter(ctx);
}

void StateMachine::update(EntityContext& ctx, float delta) {
    if (!current_state) return;

    IState* next = current_state->get_transition(ctx);
    if (next != nullptr) {
        transition_to(next, ctx);
    } else {
        current_state->update(ctx, delta);
    }
}

void StateMachine::force_transition(const char* state_name, EntityContext& ctx) {
    IState* state = require_state(state_name);
    transition_to(state, ctx);
}

bool StateMachine::is_in_state(const char* state_name) const {
    IState* state = require_state(state_name);
    if (!current_state) return false;
    return current_state == state;
}

void StateMachine::transition_to(IState* new_state, EntityContext& ctx) {
    if (current_state) {
        current_state->exit(ctx);
    }
    current_state = new_state;
    current_state->enter(ctx);
}

IState* StateMachine::require_state(const char* name) const {
    if (name == nullptr) {
        throw std::invalid_argument("State name is null");
    }

    auto it = states.find(name);
    if (it == states.end()) {
        throw std::invalid_argument("State not found: " + std::string(name));
    }
    return it->second.get();
}

} // namespace game_systems
