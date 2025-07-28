# AGL Shadow Demo

This demo showcases the AGL engine's real-time shadow mapping system.

## Features

- **Real-time shadow mapping** with 2048x2048 shadow textures
- **PCF (Percentage Closer Filtering)** for soft shadow edges
- **Interactive lighting** with animated directional light
- **Multiple objects** casting and receiving shadows
- **Ground plane** with tessellated geometry for smooth shadow reception
- **Real-time controls** for shadow parameters and visualization

## Controls

### Camera Movement
- **WASD**: Move camera (forward/back/left/right)
- **Mouse**: Look around (first-person camera)

### Demo Controls
- **F1**: Toggle ImGui UI (shows shadow parameters)
- **F2**: Toggle wireframe mode
- **F3**: Toggle shadows on/off
- **F4**: Toggle light animation
- **ESC**: Exit demo

## Running the Demo

### Quick Start
```bash
./run_shadow_demo.sh
```

### Manual Build & Run
```bash
cd build
make -j8
./bin/agl_shadow_demo
```

## Shadow System Details

The shadow system implements:

### Technical Features
- **Depth-only shadow pass**: Renders scene from light's perspective
- **Light-space matrices**: Transforms world coordinates to shadow map space
- **Shadow bias**: Prevents shadow acne artifacts
- **PCF filtering**: 3x3 sampling for smooth shadow edges
- **Orthographic projection**: For directional lights

### Shader Pipeline
1. **Shadow Map Generation**: Depth-only rendering to framebuffer
2. **Main Rendering**: Scene rendering with shadow comparison
3. **Shadow Sampling**: PCF filtering in fragment shader

### Objects in Scene
- **3 Colored Cubes**: Red, Green, Blue (cast and receive shadows)
- **2 Spheres**: Yellow, Magenta (cast and receive shadows)
- **Ground Plane**: Large tessellated plane (receives shadows)

### Light Setup
- **Directional Light**: Simulates sun/moonlight
- **Animated Movement**: Light rotates around scene (toggle with F4)
- **Shadow Camera**: Orthographic projection covering scene bounds

## Performance

- Runs at **120+ FPS** on modern hardware
- **2048x2048** shadow map resolution
- **Dual-pass rendering** (shadow + main pass)
- **Optimized depth testing** and PCF sampling

## ImGui Interface (F1)

When enabled, shows real-time controls for:
- Shadow bias adjustment
- Light position control
- Shadow map visualization
- Performance metrics
- Rendering options

The shadow system provides a solid foundation for realistic lighting in games and real-time applications.
