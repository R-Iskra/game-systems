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

### Compiling

**Windows/Linux/Mac:**

```bash
cmake -S . -B build
cmake --build build
```

Add `-DBUILD_TESTING=OFF` to compile without test executables

```bash
cmake -S . -B build -DBUILD_TESTING=OFF
```

### Running Examples

Run a specific system example on **Windows**:

```bash
./build/<system>_example.exe
```

Example:

```bash
./build/state_machine_example.exe
```

Run a specific system example on **Linux/Mac**:

```bash
./build/<system>_example
```

Example:

```bash
./build/state_machine_example
```

### Running Tests

```bash
ctest --test-dir build --output-on-failure
```