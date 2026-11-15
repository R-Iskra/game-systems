#pragma once

#include "EntityContext.h"
#include "IState.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace game_systems {

class StateMachine {
  public:
    StateMachine() = default;
    ~StateMachine() = default;

    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(StateMachine&&) = delete;

    void add_state(std::unique_ptr<IState> state);
    void start(const char* initial_state_name, EntityContext& ctx);
    void update(EntityContext& ctx, float delta);
    void force_transition(const char* state_name, EntityContext& ctx);

    const IState* get_current_state() const { return current_state; }
    bool is_in_state(const char* state_name) const;

  private:
    struct CallbackGuard {
        StateMachine& sm;
        explicit CallbackGuard(StateMachine& sm) : sm(sm) { sm.in_callback = true; }
        ~CallbackGuard() { sm.in_callback = false; }
    };

    void transition_to(IState* new_state, EntityContext& ctx);
    bool is_started() const { return current_state != nullptr; }
    bool owns(const IState* state) const;
    IState* require_state(const char* name) const;

    std::unordered_map<std::string, std::unique_ptr<IState>> states;
    IState* current_state = nullptr;
    bool in_callback = false;
};

} // namespace game_systems
