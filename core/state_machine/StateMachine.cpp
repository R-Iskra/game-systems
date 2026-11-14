#include "StateMachine.h"

#include <stdexcept>
#include <utility>

namespace game_systems {

void StateMachine::add_state(std::unique_ptr<IState> state) {
    if (state == nullptr) {
        throw std::invalid_argument("State is null");
    }

    std::string name = state->get_name();

    if (states.find(name) != states.end()) {
        throw std::invalid_argument("State already exists: " + name);
    }

    if (in_callback) {
        throw std::logic_error("Cannot add_state from inside a state callback");
    }

    if (is_started()) {
        throw std::logic_error("Cannot add_state after start");
    }

    states[name] = std::move(state);
}

void StateMachine::start(const char* initial_state_name, EntityContext& ctx) {
    IState* state = require_state(initial_state_name);

    if (in_callback) {
        throw std::logic_error("Cannot start from inside a state callback");
    }

    if (is_started()) {
        throw std::logic_error("Cannot start an already started machine");
    }

    CallbackGuard guard(*this);

    current_state = state;
    current_state->enter(ctx);
}

void StateMachine::update(EntityContext& ctx, float delta) {
    if (in_callback) {
        throw std::logic_error("Cannot update from inside a state callback");
    }

    if (!is_started()) {
        throw std::logic_error("Cannot update before start");
    }

    CallbackGuard guard(*this);

    IState* next = current_state->get_transition(ctx);

    if (next != nullptr) {
        if (!owns(next)) {
            throw std::logic_error("Cannot transition to a state this machine does not own");
        }

        transition_to(next, ctx);
    }

    current_state->update(ctx, delta);
}

void StateMachine::force_transition(const char* state_name, EntityContext& ctx) {
    IState* state = require_state(state_name);

    if (in_callback) {
        throw std::logic_error("Cannot force_transition from inside a state callback");
    }

    if (!is_started()) {
        throw std::logic_error("Cannot force_transition before start");
    }

    CallbackGuard guard(*this);

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

bool StateMachine::owns(const IState* state) const {
    for (const auto& [name, owned] : states) {
        if (owned.get() == state) return true;
    }
    return false;
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
