#include "state_machine/IState.h"

class IdleState;
class WalkState;
class JumpState;

class IdleState : public game_systems::IState {
    public:
        void enter(game_systems::EntityContext& ctx) override {
            ctx.velocity.x = 0.f;
            ctx.velocity.y = 0.f;
        }

        void update(game_systems::EntityContext& ctx, float delta) override {}

        void exit(game_systems::EntityContext& ctx) override {}

        IState* get_transition(const game_systems::EntityContext& ctx) const override {
            if (ctx.input_jump) return jump_state;
            if (ctx.input_move_left || ctx.input_move_right) return walk_state;

            return nullptr;
        }

        const char* get_name() const override { return "Idle"; }

        void set_transitions(IState* walk, IState* jump) {
            walk_state = walk;
            jump_state = jump;
        }

    private:
        IState* walk_state = nullptr;
        IState* jump_state = nullptr;
};

class WalkState : public game_systems::IState {
    public:
        void enter(game_systems::EntityContext& ctx) override {}

        void update(game_systems::EntityContext& ctx, float delta) override {
            ctx.velocity.x = ctx.input_move_right ? 100.f : -100.f;
        }

        void exit(game_systems::EntityContext& ctx) override {
            ctx.velocity.x = 0.f;
        }

        IState* get_transition(const game_systems::EntityContext& ctx) const override {
            if (ctx.input_jump) return jump_state;
            if (!ctx.input_move_left && !ctx.input_move_right) return idle_state;
            return nullptr;
        }

        const char* get_name() const override { return "Walk"; }

        void set_transitions(IState* idle, IState* jump) {
            idle_state = idle;
            jump_state = jump;
        }

    private:
        IState* idle_state = nullptr;
        IState* jump_state = nullptr;
};

class JumpState : public game_systems::IState {
    public:
        void enter(game_systems::EntityContext& ctx) override {
            ctx.velocity.y = -300.f;
        }

        void update(game_systems::EntityContext& ctx, float delta) override {
            ctx.velocity.y +=600.f * delta;
        }

        void exit(game_systems::EntityContext& ctx) override {
            ctx.velocity.y = 0.f;
        }

        IState* get_transition(const game_systems::EntityContext& ctx) const override {
            if (ctx.is_on_floor) return idle_state;
            return nullptr;
        }

        const char* get_name() const override { return "Jump"; }

        void set_transitions(IState* idle) { idle_state = idle; }

    private:
        IState* idle_state = nullptr;
};