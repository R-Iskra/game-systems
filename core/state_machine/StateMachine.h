#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "IState.h"
#include "EntityContext.h"

class StateMachine {
    public:
        StateMachine() = default;
        ~StateMachine() = default;

        void add_state(std::unique_ptr<IState> state);
        void start(const char* initial_state_name, EntityContext& ctx);
        void update(EntityContext& ctx, float delta);
        void force_transition(const char* state_name, EntityContext& ctx);

        const IState* get_current_state() const { return current_state; }
        bool is_in_state(const char* state_name) const;

    private:
        void transition_to(IState* new_state, EntityContext& ctx);

        std::unordered_map<std::string, std::unique_ptr<IState>> states;
        IState* current_state = nullptr;
};