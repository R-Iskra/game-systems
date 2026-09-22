# Event Bus

A generic, reusable event system designed to allow game systems to communicate without directly depending on one another.

## Design Decisions

- Events are identified by their C++ type using `std::type_index`, allowing different event types to coexist without requiring a common base class
- Subscribers register callbacks for a specific event type through `subscribe<TEvent>()`
- The Event Bus uses type erasure internally so callbacks for different event types can be stored in the same subscriber structure
- Multiple systems can subscribe to the same event, and all matching subscribers receive the event when it is published
- Publishing an event only notifies subscribers registered for that specific event type
- `SubscriptionHandle` provides RAII-based subscription lifetime management -- destroying a handle automatically removes its subscription
- `SubscriptionHandle` is move-only, preventing multiple handles from attempting to own the same subscription
- Subscribers can explicitly unsubscribe by releasing their `SubscriptionHandle`
- Events are passed to subscribers as `const` references, allowing systems to read event data without modifying the published event

## Note

The Event Bus is intended to decouple game systems that need to react to the same events. For example, an `EnemyDiedEvent` can be received independently by a sound system, UI system, and score system without those systems needing direct references to one another.

View `./tests/event_bus for an example implementation.

## Usage

```cpp
EventBus bus;

// create systems and subscribe them to the bus
SoundSystem sound(bus);
UISystem ui(bus);
ScoreSystem score(bus);
CameraSystem camera(bus);

// publish events
std::cout << "--- Enemy dies ---\n";
bus.publish(EnemyDiedEvent{ 1, 100 });

std::cout << "--- Player hurt ---\n";
bus.publish(PlayerHurtEvent{ 25.f });

// force unsubscribe from an event
std::cout << "--- ScoreSystem unsubscribes from EnemyDiedEvent ---\n";
score.unsubscribe();

// ScoreSystem no longer receives published event
std::cout << "--- Enemy dies again ---\n";
bus.publish(EnemyDiedEvent{ 2, 200 });
```

---
[Back to README](../README.md)

**Other Systems:**
- [State Machine](state_machine.md)
- [Event Bus](event_bus.md)