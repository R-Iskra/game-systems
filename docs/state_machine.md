# State Machine

A finite state machine that drives an entity's behavior, designed against a 2D platformer reference use case. The machine's rules don't depend on the kind of game, but its states currently take the platformer's `EntityContext`.

## Design Decisions

- `IState`: the interface each state implements (`enter`, `update`, `exit`, `get_transition`, `get_name`)
- `EntityContext`: the platformer reference context (position, velocity, floor flag, health, inputs) that every state receives. It will move out of the library in a future refactor
- The machine owns its states through `std::unique_ptr`, so states live exactly as long as the machine and can safely point at each other
- States communicate transitions by returning a pointer to the next state — no string lookups on the hot path
- `enter` and `exit` guarantee setup and teardown happen exactly once per transition, enforced by the callback rules below
- `force_transition` allows external systems to override state logic (e.g. taking damage from any state). It calls `exit` then `enter`, but not `update`; the state's first update comes from the next `update` call

## Writing Transitions

Transitions are defined per state by the developer. The machine calls `get_transition` at the start of every `update`, and the pointer it returns decides what happens next:

- `nullptr`: stay in the current state
- Another state the machine owns: transition to it
- The current state itself: restart it. It exits, then enters again
- A state the machine doesn't own: `update` throws `std::logic_error`, and nothing changes

View `./examples/state_machine` for an example implementation.

## Usage

```cpp
using namespace game_systems;

StateMachine sm;
EntityContext ctx;

// create states
auto idle = std::make_unique<IdleState>();
auto walk = std::make_unique<WalkState>();
auto jump = std::make_unique<JumpState>();

// wire transitions before transferring ownership
IdleState* idle_ptr = idle.get();
WalkState* walk_ptr = walk.get();
JumpState* jump_ptr = jump.get();

idle_ptr->set_transitions(walk_ptr, jump_ptr);
walk_ptr->set_transitions(idle_ptr, jump_ptr);
jump_ptr->set_transitions(idle_ptr);

// add to machine
sm.add_state(std::move(idle));
sm.add_state(std::move(walk));
sm.add_state(std::move(jump));

// start and update
sm.start("Idle", ctx);
sm.update(ctx, 0.016f);
```

## Update Order

Each call to `update` does three things, in order:

1. Calls the current state's `get_transition`
2. If it returned a state, calls `exit` on the current state, then `enter` on the new one
3. Calls `update` on whichever state is now current

At most one transition happens per `update`; a newly entered state isn't checked until the next call.

Why this order:

- A newly entered state updates on the same frame, so a transition never costs a frame of lag. In the example, Walk's velocity is already `(100, 0)` on the frame it's entered
- A state only updates when the conditions it just checked say it should be running

Trade-off: a transition caused by the state's own `update` (e.g. a timer running out) is noticed on the next call, up to one frame late.

## Lifecycle

| Call | Before `start` | After `start` |
| --- | --- | --- |
| `add_state` | Adds the state | Throws `std::logic_error` |
| `start` | Enters the named state | Throws `std::logic_error` |
| `update` | Throws `std::logic_error` | Checks, transitions if needed, then updates |
| `force_transition` | Throws `std::logic_error` | Exits the current state, enters the named one |
| `get_current_state` | Returns `nullptr` | Returns the current state |
| `is_in_state` | Returns `false` | Returns whether the named state is current |

- **`start` only enters** the named state. It doesn't check transitions or update, because there is no frame yet. Call `update` for the first frame.
- **Forcing a transition to the current state restarts it:** it exits, then enters again, the same as `get_transition` returning the current state. For example, a second hit can restart a Hurt state's stun.
- **Why setup is locked after `start`:** every state is known when the entity is created, and loosening the rule later breaks nobody, while tightening it later would.

## Errors

| Call | Condition | Exception |
| --- | --- | --- |
| `add_state` | A null state | `std::invalid_argument` |
| `add_state` | A duplicate name | `std::invalid_argument` |
| `add_state` | Called from inside a callback | `std::logic_error` |
| `add_state` | Called after `start` | `std::logic_error` |
| `start` | A null or unknown name | `std::invalid_argument` |
| `start` | Called from inside a callback | `std::logic_error` |
| `start` | The machine is already started | `std::logic_error` |
| `update` | Called from inside a callback | `std::logic_error` |
| `update` | Called before `start` | `std::logic_error` |
| `update` | `get_transition` returned a state the machine doesn't own | `std::logic_error` |
| `force_transition` | A null or unknown name | `std::invalid_argument` |
| `force_transition` | Called from inside a callback | `std::logic_error` |
| `force_transition` | Called before `start` | `std::logic_error` |
| `is_in_state` | A null or unknown name | `std::invalid_argument` |

- **Check order:** arguments, then callbacks, then lifecycle. When several things are wrong, the first failing check is the one reported.
- **Catching:** `std::invalid_argument` derives from `std::logic_error`, so catching `std::logic_error` catches every misuse error. All of them indicate a bug in the calling code.

## Callbacks and Reentrancy

A callback is any state function the machine calls: `enter`, `update`, `exit`, and `get_transition`. While one is running, calls back into the machine are restricted:

- `start`, `update`, `force_transition`, and `add_state` throw `std::logic_error`
- `is_in_state` and `get_current_state` are allowed; asking where the machine is can't disturb it

Why: callbacks often reach other systems. Suppose Jump's `update` publishes an event, and a damage system's listener calls `force_transition("Hurt")`. Without this rule, Jump is exited while its own `update` is still running, and the rest of the `update` runs on an entity that has already left Jump. Inside an `exit` it's worse: the outer transition resumes and overwrites the current state, so Hurt is entered but never exited. Rejecting the call turns that silent corruption into an immediate error.

What to do instead:

- A state that wants to leave returns the target from `get_transition`
- An outside system that needs to force a change calls `force_transition` after `update` has returned, or records the event in the context (e.g. lowering `health`) so a state's `get_transition` reacts on the next `update`

The restriction lifts when the outer call returns, even if a callback throws, so an exception never leaves the machine locked.

## Open Questions

- **Callbacks that throw:** callbacks are expected not to throw. If one does, the exception reaches the caller and the machine is not locked, but which state it's left in is unspecified.

---
[Back to README](../README.md)

**Other Systems:**
- [State Machine](state_machine.md)
- [Event Bus](event_bus.md)
