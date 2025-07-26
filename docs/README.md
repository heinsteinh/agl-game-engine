# AGL Game Engine Documentation

This directory contains comprehensive documentation for the AGL Game Engine camera system and other components.

## 📄 Available Documentation

### Camera System Documentation

- **[CAMERA_DOCUMENTATION.md](CAMERA_DOCUMENTATION.md)** - Complete camera system guide
  - Comprehensive API reference
  - Usage examples and code snippets
  - Configuration guides for different game types
  - Troubleshooting and best practices

### Doxygen API Documentation

To generate full API documentation with Doxygen:

#### Prerequisites
Install Doxygen on your system:

```bash
# macOS
brew install doxygen

# Ubuntu/Debian
sudo apt-get install doxygen

# Windows
# Download from https://www.doxygen.nl/download.html
```

#### Generate Documentation

```bash
# From the root directory
./generate_docs.sh

# Or manually
doxygen Doxyfile
```

This will create:
- `docs/doxygen/html/index.html` - Main documentation page
- Complete API reference for all classes
- Cross-referenced source code
- Search functionality

## 📚 Documentation Content

### Camera System Features Documented

1. **Core Camera Class (`agl::Camera`)**
   - 3D transformations and matrix calculations
   - Perspective and orthographic projections
   - Euler angle rotation system
   - Input processing for movement and mouse look
   - Screen-to-world ray casting
   - View frustum culling

2. **Camera Controller (`agl::CameraController`)**
   - Multiple camera modes (first-person, third-person, spectator, fixed)
   - Shooter game mechanics (sprint, crouch, aim)
   - Camera shake effects
   - Dynamic FOV transitions
   - Smooth movement interpolation
   - Configurable settings system

3. **Advanced Features**
   - Integration examples with game loops
   - Performance optimization techniques
   - Troubleshooting common issues
   - Configuration for different game genres

## 🎯 Quick Access

- **New to AGL?** Start with [CAMERA_DOCUMENTATION.md](CAMERA_DOCUMENTATION.md)
- **API Reference?** Generate Doxygen docs with `./generate_docs.sh`
- **Examples?** Check the sandbox demos in `sandbox/src/`
- **Issues?** See the troubleshooting section in the camera documentation

## 🔗 Related Files

- `Doxyfile` - Doxygen configuration
- `generate_docs.sh` - Documentation generation script
- `gamelib/include/Camera.h` - Core camera class with Doxygen comments
- `gamelib/include/CameraController.h` - Camera controller with Doxygen comments
- `sandbox/src/*_demo.cpp` - Usage examples and demonstrations

---

*For the latest documentation, see [GitHub Pages](https://heinsteinh.github.io/agl-game-engine/)*
