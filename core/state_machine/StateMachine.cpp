#include "StateMachine.h"

#include <stdexcept>

void StateMachine::add_state(std::unique_ptr<IState> state) {
    std::string name = state->get_name();
    states[name] = std::move(state);
}

void StateMachine::start(const char* initial_state_name, EntityContext& ctx) {
    auto it = states.find(initial_state_name);
    if (it == states.end()) {
        throw std::runtime_error("State not found: " + std::string(initial_state_name));
    }
    current_state = it->second.get();
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
    auto it = states.find(state_name);
    if (it == states.end()) {
        throw std::runtime_error("State not found: " + std::string(state_name));
    }
    transition_to(it->second.get(), ctx);
}

void StateMachine::transition_to(IState* new_state, EntityContext& ctx) {
    if (current_state) {
        current_state->exit(ctx);
    }
    current_state = new_state;
    current_state->enter(ctx);
}

bool StateMachine::is_in_state(const char* state_name) const {
    if (!current_state) return false;
    return std::string(current_state->get_name()) == state_name;
}