# Game Systems

A collection of reusable, engine-agnostic game systems written in C++.

## Systems

- [x] [State Machine](docs/state_machine.md)
- [x] [Event Bus](docs/event_bus.md)
- [ ] Pathfinding
- [ ] Behavior Tree
- [ ] Perception System
- [ ] Inventory System
- [ ] Save/Load System

## Building

### Requirements
- GCC 13+
- CMake 3.20+
- C++17 or higher

### Compile and Run

**Windows:**
```bash
cmake -S . -B build
cmake --build build
./build/test_state_machine.exe
```

**Linux/Mac:**
```bash
cmake -S . -B build
cmake --build build
./build/test_state_machine
```