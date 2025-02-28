# Interactive Particle System

Interactive particle system with multiple emitter types, dynamic force fields, and particle interactions using C++23 and SDL2. Explore colorful visual effects, physics simulation, and multithreaded performance optimizations.

## Demo

![Demo](/resources/particles-1.gif)

## Features

- **Multiple Emitter Types**: Fountain, explosion, snow, spiral patterns
- **Interactive Controls**: Mouse-controlled force fields
- **Physics Simulation**: Gravity, attraction/repulsion, particle interactions
- **Visual Effects**: Dynamic colors, glowing force fields, rainbow mode
- **Optimized Performance**: Multithreaded, spatial partitioning

## Controls

- **Mouse**: Control force field position
- **Left Click**: Create burst at cursor
- **Space**: Toggle attraction/repulsion
- **F**: Toggle force field on/off
- **1-4**: Switch emitter types
- **C**: Toggle colorful mode
- **B**: Toggle dynamic background
- **I**: Toggle particle interaction
- **R**: Reset system
- **Q/ESC**: Quit

## Build and Run

```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run
./particle_system
```

## Requirements

- C++23 compatible compiler
- SDL2 development libraries
- CMake 3.16+
