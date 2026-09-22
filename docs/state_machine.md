# State Machine

A generic, reusable finite state machine designed to drive entities.

## Design Decisions

- `IState` represents a blank slate for actual states to be built off of
- `EntityContext` represents a blank slate for entities to be built off of
- The state machine owns states via `std::unique_ptr` rather than the entity, keeping it generic and reusable across any entity type
- States communicate transitions by returning a pointer to the next state — no string lookups on the hot path
- `enter()` and `exit()` guarantee setup and teardown happen exactly once per transition
- `force_transition()` allows external systems to override state logic (e.g. taking damage from any state)

## Note

Transitions are defined per state by the developer. Each concrete state implements `get_transition` and returns a pointer to the next state when its transition condition is met, or `nullptr` to remain in the current state. View `./tests/state_machine` for an example implementation.

## Usage

```cpp
StateMachine sm;

// create states
auto idle = std::make_unique<IdleState>();
auto walk = std::make_unique<WalkState>();
auto jump = std::make_unique<JumpState>();

// wire transitions before transferring ownership
IdleState* idle_ptr = idle.get();
WalkState* walk_ptr = walk.get();
JumpState* jump_ptr = jump.get();

idle_ptr->set_walk_state(walk_ptr);
walk_ptr->set_transitions(idle_ptr, jump_ptr);
jump_ptr->set_idle_state(idle_ptr);

// add to machine
sm.add_state(std::move(idle));
sm.add_state(std::move(walk));
sm.add_state(std::move(jump));

// start and update
sm.start("Idle", ctx);
sm.update(ctx, delta);
```

---
[Back to README](../README.md)

**Other Systems:**
- [State Machine](state_machine.md)
- [Event Bus](event_bus.md)