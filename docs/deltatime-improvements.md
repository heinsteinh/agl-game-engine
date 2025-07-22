# DeltaTime Improvements in AGL Game Engine

## Overview
The deltaTime system has been significantly improved to provide better frame rate consistency, performance monitoring, and frame-rate independent gameplay.

## Key Improvements

### 1. **Fixed DeltaTime Calculation Bug**
- **Before**: `m_lastFrameTime = currentTime - m_lastFrameTime; m_lastFrameTime = currentTime;` (incorrect logic)
- **After**: Proper calculation that computes actual time difference between frames

### 2. **DeltaTime Clamping**
- **Purpose**: Prevents "spiral of death" when frame rate drops drastically
- **Implementation**: Caps deltaTime at 1/20th second (20 FPS minimum)
- **Benefit**: Ensures game logic remains stable even during frame drops

### 3. **FPS Tracking and Monitoring**
- Real-time FPS calculation
- Average frame time computation
- Performance logging every 5 seconds

### 4. **Enhanced Timing API**
New getter methods available:
```cpp
float GetDeltaTime() const;        // Current frame's delta time
float GetFPS() const;              // Current FPS
float GetAverageDeltaTime() const; // Average delta time over 1 second
```

### 5. **ImGui Integration**
- Real-time FPS display
- Frame time graph visualization
- Performance metrics in debug window

## Usage Examples

### Frame-Rate Independent Movement
```cpp
void OnUpdate(float deltaTime) override {
    float speed = 100.0f; // units per second
    position += speed * deltaTime; // Always moves 100 units/second regardless of FPS
}
```

### Timing-Based Events
```cpp
void OnUpdate(float deltaTime) override {
    static float timer = 0.0f;
    timer += deltaTime;
    
    if (timer >= 1.0f) { // Every second
        DoSomething();
        timer = 0.0f;
    }
}
```

### Performance Monitoring
```cpp
void OnImGuiRender() override {
    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", GetFPS());
    ImGui::Text("Frame Time: %.3f ms", GetDeltaTime() * 1000.0f);
    ImGui::End();
}
```

## Technical Details

### Constants
- `MAX_DELTA_TIME = 1.0f / 20.0f`: Maximum allowed delta time (50ms)
- `FPS_UPDATE_INTERVAL = 1.0f`: FPS calculation update frequency

### Frame Time Accumulation
The system accumulates frame times over 1-second intervals to calculate:
- Average FPS
- Average frame time
- Smooth performance metrics

### Logging
- Core engine logs performance metrics every 5 seconds
- Uses AGL_CORE_TRACE level to avoid spam in release builds

## Benefits

1. **Stability**: No more frame rate death spirals
2. **Consistency**: Frame-rate independent gameplay
3. **Monitoring**: Real-time performance feedback
4. **Debugging**: Visual frame time graphs in ImGui
5. **Profiling**: Automatic performance logging

## Migration Notes

If you have existing code using the old deltaTime, it should continue working without changes. The main difference is that deltaTime values are now clamped and more accurate.

For new projects, use the enhanced API:
- `GetDeltaTime()` for current frame delta
- `GetFPS()` for current FPS
- `GetAverageDeltaTime()` for smoothed timing

## Future Enhancements

Potential future improvements:
- Configurable frame rate limiting
- Multiple timing modes (fixed timestep, variable timestep)
- Frame pacing for VSync scenarios
- Historical performance data storage
