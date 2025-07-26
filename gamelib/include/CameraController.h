/**
 * @file CameraController.h
 * @brief High-level camera controller for the AGL Game Engine
 * @author AGL Team
 * @version 1.0
 * @date 2025
 * 
 * This file contains the CameraController class which provides high-level camera control
 * with game-specific features including multiple camera modes, input handling, movement
 * smoothing, shooter game mechanics (sprint, crouch, aim), camera shake effects,
 * and dynamic field of view transitions.
 * 
 * The CameraController works with the Camera class to provide a complete camera system
 * suitable for modern game development, particularly first-person and third-person games.
 */

#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "Camera.h"
#include "input.h"
#include <memory>

namespace agl {

    /**
     * @brief Enumeration of camera control modes
     * 
     * Defines different camera behavior modes that determine how the camera
     * responds to input and how it's positioned relative to the game world.
     */
    enum class CameraMode {
        FirstPerson,    ///< Standard first-person shooter camera with direct control
        ThirdPerson,    ///< Third-person camera that follows a target with configurable offset
        Spectator,      ///< Free-flying camera for debugging or level editing
        Fixed           ///< Stationary camera that can optionally track targets
    };

    /**
     * @brief Camera controller configuration settings
     * 
     * This structure contains all configurable parameters for the CameraController,
     * allowing fine-tuning of camera behavior for different game types and player preferences.
     * 
     * @example Basic Configuration
     * @code
     * agl::CameraSettings settings;
     * settings.movementSpeed = 8.0f;          // Fast movement
     * settings.mouseSensitivity = 0.15f;      // Good for aiming
     * settings.defaultFOV = 75.0f;            // Standard FOV
     * settings.aimFOV = 50.0f;                // Zoom when aiming
     * controller.SetSettings(settings);
     * @endcode
     */
    struct CameraSettings {
        // ========== Movement Settings ==========
        
        /**
         * @brief Base movement speed in units per second
         * 
         * Controls how fast the camera moves when using keyboard input.
         * This is the base speed that gets modified by sprint/crouch multipliers.
         */
        float movementSpeed = 5.0f;

        /**
         * @brief Sprint speed multiplier
         * 
         * Multiplier applied to movement speed when sprinting.
         * Example: If movementSpeed = 5.0 and sprintMultiplier = 2.0,
         * sprint speed = 10.0 units/second.
         */
        float sprintMultiplier = 2.0f;

        /**
         * @brief Crouch speed multiplier
         * 
         * Multiplier applied to movement speed when crouching.
         * Should be less than 1.0 to slow down movement while crouching.
         */
        float crouchMultiplier = 0.5f;

        // ========== Mouse Settings ==========
        
        /**
         * @brief Mouse sensitivity multiplier
         * 
         * Controls how sensitive the camera is to mouse movement.
         * Lower values = smoother/less sensitive, higher values = more responsive.
         * Typical range: 0.05 to 0.3
         */
        float mouseSensitivity = 0.1f;

        /**
         * @brief Whether to invert Y-axis mouse movement
         * 
         * When true, moving mouse up rotates camera down and vice versa.
         * This is a common preference setting in games.
         */
        bool invertY = false;

        // ========== Smoothing Settings ==========
        
        /**
         * @brief Movement smoothing factor
         * 
         * Controls how much smoothing is applied to camera movement.
         * 0.0 = no smoothing (instant), 1.0 = maximum smoothing.
         * Higher values create smoother but less responsive movement.
         */
        float movementSmoothing = 0.1f;

        /**
         * @brief Rotation smoothing factor
         * 
         * Controls how much smoothing is applied to camera rotation.
         * Similar to movementSmoothing but for mouse look.
         * Lower values recommended for responsive aiming.
         */
        float rotationSmoothing = 0.1f;

        // ========== Third Person Settings ==========
        
        /**
         * @brief Distance from target in third-person mode
         * 
         * How far behind/away from the target the camera is positioned
         * when in ThirdPerson mode.
         */
        float thirdPersonDistance = 5.0f;

        /**
         * @brief Height offset from target in third-person mode
         * 
         * How much higher than the target the camera is positioned.
         * Positive values place camera above target.
         */
        float thirdPersonHeight = 2.0f;

        /**
         * @brief Additional offset for third-person camera positioning
         * 
         * Extra positional offset applied to the third-person camera.
         * Useful for over-the-shoulder or side-view camera angles.
         */
        glm::vec3 thirdPersonOffset = glm::vec3(0.0f, 0.0f, 0.0f);

        // ========== Camera Constraints ==========
        
        /**
         * @brief Whether to constrain pitch rotation
         * 
         * When true, prevents the camera from rotating past vertical limits
         * to avoid gimbal lock and disorientation.
         */
        bool constrainPitch = true;

        /**
         * @brief Minimum pitch angle in degrees
         * 
         * Lowest angle the camera can look (looking down).
         * Typically -89° to prevent flipping over.
         */
        float minPitch = -89.0f;

        /**
         * @brief Maximum pitch angle in degrees
         * 
         * Highest angle the camera can look (looking up).
         * Typically 89° to prevent flipping over.
         */
        float maxPitch = 89.0f;

        // ========== Field of View Settings ==========
        
        /**
         * @brief Default field of view in degrees
         * 
         * Standard FOV used when not sprinting or aiming.
         * Typical values: 60-90 degrees.
         */
        float defaultFOV = 75.0f;

        /**
         * @brief Field of view when sprinting
         * 
         * FOV used during sprint to create speed sensation.
         * Usually higher than default to create "speed effect".
         */
        float sprintFOV = 85.0f;

        /**
         * @brief Field of view when aiming
         * 
         * FOV used when aiming to create zoom effect.
         * Usually lower than default for precision aiming.
         */
        float aimFOV = 50.0f;

        /**
         * @brief Speed of FOV transitions
         * 
         * How quickly the FOV changes between different states.
         * Higher values = faster transitions.
         */
        float fovTransitionSpeed = 5.0f;

        // ========== Camera Shake Settings ==========
        
        /**
         * @brief Global camera shake intensity multiplier
         * 
         * Multiplier applied to all camera shake effects.
         * Can be used as a global shake sensitivity setting.
         */
        float shakeIntensity = 1.0f;

        /**
         * @brief Camera shake damping factor
         * 
         * How quickly camera shake effects fade out.
         * Higher values = faster decay.
         */
        float shakeDamping = 5.0f;
    };

    /**
     * @brief High-level camera controller for game development
     * 
     * The CameraController class provides a complete camera control system designed
     * for modern game development. It wraps the core Camera class with additional
     * functionality including:
     * 
     * - Multiple camera modes (first-person, third-person, spectator, fixed)
     * - Input handling with customizable key bindings
     * - Smooth movement and rotation interpolation
     * - Shooter game mechanics (sprint, crouch, aim)
     * - Dynamic field of view transitions
     * - Camera shake effects
     * - Configurable settings for different game types
     * 
     * The controller automatically handles input processing and camera updates,
     * making it easy to integrate professional camera behavior into games.
     * 
     * @example Basic Usage
     * @code
     * // Create camera and controller
     * auto camera = std::make_shared<agl::Camera>(glm::vec3(0, 2, 5));
     * auto controller = std::make_unique<agl::CameraController>(camera);
     * controller->Initialize(inputSystem);
     * 
     * // Configure for shooter game
     * agl::CameraSettings settings;
     * settings.movementSpeed = 8.0f;
     * settings.aimFOV = 45.0f;
     * controller->SetSettings(settings);
     * 
     * // In game loop
     * controller->Update(deltaTime);
     * @endcode
     * 
     * @example Advanced Configuration
     * @code
     * // Setup for third-person adventure game
     * controller->SetMode(agl::CameraMode::ThirdPerson);
     * controller->SetTarget(playerPosition);
     * 
     * agl::CameraSettings& settings = controller->GetSettings();
     * settings.thirdPersonDistance = 8.0f;
     * settings.thirdPersonHeight = 3.0f;
     * settings.movementSmoothing = 0.2f;  // Smoother for third-person
     * @endcode
     * 
     * @note The CameraController requires a valid Camera instance and Input system
     *       to function properly. Always call Initialize() before Update().
     * 
     * @see Camera for core camera functionality
     * @see CameraSettings for configuration options
     * @see CameraMode for available camera modes
     */
    class CameraController {
    public:
        // ========== Constructors ==========
        
        /**
         * @brief Default constructor
         * 
         * Creates a camera controller without an associated camera.
         * You must call SetCamera() before using the controller.
         * 
         * @note Requires SetCamera() and Initialize() to be called before Update()
         */
        CameraController();

        /**
         * @brief Constructor with camera
         * 
         * Creates a camera controller associated with the specified camera.
         * The controller will control the provided camera instance.
         * 
         * @param camera Shared pointer to the camera to control
         * 
         * @note Still requires Initialize() to be called before Update()
         */
        explicit CameraController(std::shared_ptr<Camera> camera);

        /**
         * @brief Destructor
         * 
         * Default destructor. The controller uses RAII and smart pointers
         * so no manual cleanup is required.
         */
        ~CameraController() = default;

        // ========== Initialization ==========

        /**
         * @brief Initialize the controller with an input system
         * 
         * Associates the controller with an input system for processing
         * keyboard and mouse input. Must be called before Update().
         * 
         * @param input Pointer to the input system to use
         * 
         * @note The input system must remain valid for the lifetime of the controller
         * 
         * @example
         * @code
         * controller->Initialize(game.GetInput());
         * @endcode
         */
        void Initialize(Input* input);

        // ========== Update ==========

        /**
         * @brief Update the camera controller
         * 
         * Main update function that processes input, applies camera logic,
         * and updates the associated camera. Should be called once per frame.
         * 
         * @param deltaTime Time elapsed since last frame in seconds
         * 
         * @note This function handles:
         *       - Input processing (keyboard/mouse)
         *       - Camera mode-specific logic
         *       - Smoothing and interpolation
         *       - FOV transitions
         *       - Camera shake updates
         * 
         * @example
         * @code
         * // In game loop
         * void OnUpdate(float deltaTime) override {
         *     cameraController->Update(deltaTime);
         *     // ... rest of game logic
         * }
         * @endcode
         */
        void Update(float deltaTime);

        // ========== Camera Management ==========

        /**
         * @brief Set the camera to control
         * 
         * Associates the controller with a different camera instance.
         * The controller will immediately start controlling the new camera.
         * 
         * @param camera Shared pointer to the new camera to control
         * 
         * @example
         * @code
         * auto newCamera = std::make_shared<agl::Camera>(newPosition);
         * controller->SetCamera(newCamera);
         * @endcode
         */
        void SetCamera(std::shared_ptr<Camera> camera);

        /**
         * @brief Get the currently controlled camera
         * 
         * Returns a shared pointer to the camera currently being controlled.
         * Useful for accessing camera properties directly.
         * 
         * @return Shared pointer to the controlled camera
         * 
         * @example
         * @code
         * auto camera = controller->GetCamera();
         * glm::mat4 viewMatrix = camera->GetViewMatrix();
         * @endcode
         */
        std::shared_ptr<Camera> GetCamera() const { return m_camera; }

        // ========== Mode Management ==========

        /**
         * @brief Set the camera control mode
         * 
         * Changes how the camera behaves and responds to input.
         * Each mode provides different camera control schemes suitable
         * for different game types and situations.
         * 
         * @param mode The new camera mode to use
         * 
         * @see CameraMode for available modes and their descriptions
         * 
         * @example
         * @code
         * // Switch to first-person for shooting segments
         * controller->SetMode(agl::CameraMode::FirstPerson);
         * 
         * // Switch to third-person for exploration
         * controller->SetMode(agl::CameraMode::ThirdPerson);
         * controller->SetTarget(playerPosition);
         * @endcode
         */
        void SetMode(CameraMode mode);

        /**
         * @brief Get the current camera mode
         * 
         * Returns the currently active camera control mode.
         * 
         * @return Current camera mode
         * 
         * @example
         * @code
         * if (controller->GetMode() == agl::CameraMode::ThirdPerson) {
         *     // Update target position for third-person mode
         *     controller->SetTarget(player.GetPosition());
         * }
         * @endcode
         */
        CameraMode GetMode() const { return m_mode; }

        // ========== Settings Management ==========

        /**
         * @brief Set the camera controller settings
         * 
         * Applies a new set of configuration settings to the controller.
         * This immediately updates the controller's behavior.
         * 
         * @param settings New settings to apply
         * 
         * @example
         * @code
         * agl::CameraSettings settings;
         * settings.movementSpeed = 10.0f;
         * settings.mouseSensitivity = 0.2f;
         * settings.aimFOV = 40.0f;
         * controller->SetSettings(settings);
         * @endcode
         */
        void SetSettings(const CameraSettings& settings);

        /**
         * @brief Get mutable reference to settings
         * 
         * Returns a reference to the controller's settings structure,
         * allowing direct modification of individual settings.
         * 
         * @return Mutable reference to camera settings
         * 
         * @example
         * @code
         * // Modify individual settings
         * agl::CameraSettings& settings = controller->GetSettings();
         * settings.movementSpeed = 8.0f;
         * settings.sprintMultiplier = 2.5f;
         * @endcode
         */
        CameraSettings& GetSettings() { return m_settings; }

        /**
         * @brief Get const reference to settings
         * 
         * Returns a const reference to the controller's settings structure
         * for read-only access.
         * 
         * @return Const reference to camera settings
         * 
         * @example
         * @code
         * const auto& settings = controller->GetSettings();
         * float currentSpeed = settings.movementSpeed;
         * @endcode
         */
        const CameraSettings& GetSettings() const { return m_settings; }

        // ========== Position and Target Management ==========

        /**
         * @brief Set the target position for third-person mode
         * 
         * Sets the world position that the camera should focus on or follow
         * when in ThirdPerson mode. The camera will maintain the configured
         * distance and height offset from this target.
         * 
         * @param target World position to target
         * 
         * @note This only affects ThirdPerson mode behavior
         * 
         * @example
         * @code
         * // In game loop for third-person camera
         * glm::vec3 playerPos = player.GetPosition();
         * controller->SetTarget(playerPos);
         * @endcode
         */
        void SetTarget(const glm::vec3& target);

        /**
         * @brief Set the camera position directly
         * 
         * Directly sets the camera's world position, overriding any
         * mode-specific positioning logic.
         * 
         * @param position New world position for the camera
         * 
         * @note This may be overridden by camera mode logic on the next update
         * 
         * @example
         * @code
         * // Position camera for cutscene
         * controller->SetPosition(glm::vec3(10, 5, 0));
         * @endcode
         */
        void SetPosition(const glm::vec3& position);

        /**
         * @brief Get the current target position
         * 
         * Returns the current target position used for third-person mode.
         * 
         * @return Current target position
         */
        glm::vec3 GetTarget() const { return m_target; }

        // ========== Shooter Game Features ==========

        /**
         * @brief Start aiming mode
         * 
         * Activates aiming mode, which typically:
         * - Reduces field of view for zoom effect
         * - May reduce mouse sensitivity for precision
         * - Can affect movement speed
         * 
         * @note Usually triggered by right mouse button press
         * 
         * @example
         * @code
         * if (input->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
         *     controller->StartAiming();
         * }
         * @endcode
         */
        void StartAiming();

        /**
         * @brief Stop aiming mode
         * 
         * Deactivates aiming mode, returning FOV and other settings
         * to their default values.
         * 
         * @example
         * @code
         * if (input->IsMouseButtonReleased(GLFW_MOUSE_BUTTON_RIGHT)) {
         *     controller->StopAiming();
         * }
         * @endcode
         */
        void StopAiming();

        /**
         * @brief Check if currently aiming
         * 
         * Returns whether the camera is currently in aiming mode.
         * 
         * @return true if aiming, false otherwise
         * 
         * @example
         * @code
         * if (controller->IsAiming()) {
         *     // Show crosshair, reduce recoil, etc.
         * }
         * @endcode
         */
        bool IsAiming() const { return m_isAiming; }

        /**
         * @brief Start sprinting mode
         * 
         * Activates sprinting mode, which typically:
         * - Increases movement speed
         * - May increase field of view for speed sensation
         * - Can affect camera shake or bob
         * 
         * @note Usually triggered by holding shift key
         * 
         * @example
         * @code
         * if (input->IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
         *     controller->StartSprinting();
         * }
         * @endcode
         */
        void StartSprinting();

        /**
         * @brief Stop sprinting mode
         * 
         * Deactivates sprinting mode, returning movement speed and
         * FOV to their default values.
         */
        void StopSprinting();

        /**
         * @brief Check if currently sprinting
         * 
         * Returns whether the camera is currently in sprinting mode.
         * 
         * @return true if sprinting, false otherwise
         * 
         * @example
         * @code
         * if (controller->IsSprinting()) {
         *     // Play sprint sound, show stamina drain, etc.
         * }
         * @endcode
         */
        bool IsSprinting() const { return m_isSprinting; }

        /**
         * @brief Start crouching mode
         * 
         * Activates crouching mode, which typically:
         * - Reduces movement speed
         * - May lower camera position
         * - Can improve accuracy in shooter games
         * 
         * @note Usually triggered by holding control key
         */
        void StartCrouching();

        /**
         * @brief Stop crouching mode
         * 
         * Deactivates crouching mode, returning movement speed and
         * camera position to their default values.
         */
        void StopCrouching();

        /**
         * @brief Check if currently crouching
         * 
         * Returns whether the camera is currently in crouching mode.
         * 
         * @return true if crouching, false otherwise
         */
        bool IsCrouching() const { return m_isCrouching; }

        // ========== Camera Effects ==========

        /**
         * @brief Add camera shake effect
         * 
         * Adds a camera shake effect with the specified intensity and duration.
         * Multiple shake effects can be active simultaneously and will combine.
         * Useful for weapon fire, explosions, impacts, etc.
         * 
         * @param intensity Shake strength (0.0 = no shake, 1.0+ = strong shake)
         * @param duration How long the shake should last in seconds
         * 
         * @note Intensity is multiplied by the global shakeIntensity setting
         * 
         * @example
         * @code
         * // Add shake for weapon fire
         * controller->AddShake(0.3f, 0.2f);
         * 
         * // Add strong shake for explosion
         * controller->AddShake(1.0f, 1.5f);
         * @endcode
         */
        void AddShake(float intensity, float duration = 0.5f);

        /**
         * @brief Clear all camera shake effects
         * 
         * Immediately stops all camera shake effects and returns
         * the camera to its unshaken position.
         * 
         * @example
         * @code
         * // Stop shake when entering menu
         * if (gameState == Menu) {
         *     controller->ClearShake();
         * }
         * @endcode
         */
        void ClearShake();

        // ========== Field of View Control ==========

        /**
         * @brief Set the target field of view
         * 
         * Sets the field of view that the camera should transition to.
         * The transition speed is controlled by fovTransitionSpeed setting.
         * 
         * @param fov Target field of view in degrees
         * 
         * @note This overrides mode-specific FOV until the next mode change
         * 
         * @example
         * @code
         * // Zoom in for binoculars
         * controller->SetFOV(20.0f);
         * 
         * // Return to normal
         * controller->SetFOV(75.0f);
         * @endcode
         */
        void SetFOV(float fov);

        /**
         * @brief Get the current field of view
         * 
         * Returns the camera's current field of view. This may be different
         * from the target FOV if a transition is in progress.
         * 
         * @return Current field of view in degrees
         * 
         * @example
         * @code
         * float currentFOV = controller->GetCurrentFOV();
         * ui->DisplayFOV(currentFOV);
         * @endcode
         */
        float GetCurrentFOV() const { return m_currentFOV; }

        // ========== Input Processing ==========

        /**
         * @brief Process keyboard input for camera movement
         * 
         * Internal function that handles keyboard-based camera movement.
         * Called automatically by Update() but can be called separately
         * for custom input handling.
         * 
         * @param deltaTime Time elapsed since last frame
         * 
         * @note Usually not called directly; Update() handles this automatically
         */
        void ProcessKeyboard(float deltaTime);

        /**
         * @brief Process mouse input for camera rotation
         * 
         * Internal function that handles mouse-based camera rotation.
         * Called automatically by Update() but can be called separately
         * for custom input handling.
         * 
         * @param deltaTime Time elapsed since last frame
         * 
         * @note Usually not called directly; Update() handles this automatically
         */
        void ProcessMouse(float deltaTime);

        /**
         * @brief Process mouse scroll input for zoom
         * 
         * Handles mouse scroll wheel input for camera zoom functionality.
         * Called automatically by Update() when scroll input is detected.
         * 
         * @param yoffset Vertical scroll offset
         * 
         * @note Usually not called directly; Update() handles this automatically
         */
        void ProcessMouseScroll(float yoffset);

        // ========== Utility Functions ==========

        /**
         * @brief Reset camera to default state
         * 
         * Resets the camera and controller to their initial state:
         * - Camera position and rotation
         * - FOV to default value
         * - Clears all active states (aiming, sprinting, etc.)
         * - Stops camera shake
         * - Resets smoothing variables
         * 
         * @example
         * @code
         * // Reset button in options menu
         * if (resetButtonPressed) {
         *     controller->Reset();
         * }
         * @endcode
         */
        void Reset();

        // ========== Control Flags ==========

        /**
         * @brief Enable or disable input processing
         * 
         * Controls whether the controller processes input from the input system.
         * Useful for disabling camera control during cutscenes, menus, etc.
         * 
         * @param enabled true to enable input, false to disable
         * 
         * @example
         * @code
         * // Disable camera during cutscene
         * if (inCutscene) {
         *     controller->SetInputEnabled(false);
         * } else {
         *     controller->SetInputEnabled(true);
         * }
         * @endcode
         */
        void SetInputEnabled(bool enabled) { m_inputEnabled = enabled; }

        /**
         * @brief Check if input processing is enabled
         * 
         * Returns whether the controller is currently processing input.
         * 
         * @return true if input is enabled, false otherwise
         */
        bool IsInputEnabled() const { return m_inputEnabled; }

        /**
         * @brief Enable or disable movement smoothing
         * 
         * Controls whether smoothing is applied to camera movement and rotation.
         * Can be disabled for more responsive controls or enabled for smoother motion.
         * 
         * @param enabled true to enable smoothing, false for immediate response
         * 
         * @example
         * @code
         * // Disable smoothing for competitive gameplay
         * controller->SetSmoothingEnabled(false);
         * 
         * // Enable smoothing for cinematic feel
         * controller->SetSmoothingEnabled(true);
         * @endcode
         */
        void SetSmoothingEnabled(bool enabled) { m_smoothingEnabled = enabled; }

        /**
         * @brief Check if smoothing is enabled
         * 
         * Returns whether movement and rotation smoothing is currently enabled.
         * 
         * @return true if smoothing is enabled, false otherwise
         */
        bool IsSmoothingEnabled() const { return m_smoothingEnabled; }

    private:
        // ========== Member Variables ==========
        
        std::shared_ptr<Camera> m_camera;       ///< The camera being controlled
        Input* m_input;                         ///< Input system for processing user input
        CameraMode m_mode;                      ///< Current camera control mode
        CameraSettings m_settings;              ///< Configuration settings

        glm::vec3 m_target;                     ///< Target position for third-person mode

        // Camera state flags
        bool m_isAiming;                        ///< Whether currently aiming
        bool m_isSprinting;                     ///< Whether currently sprinting
        bool m_isCrouching;                     ///< Whether currently crouching
        bool m_inputEnabled;                    ///< Whether input processing is enabled
        bool m_smoothingEnabled;                ///< Whether smoothing is enabled

        // Smoothing variables
        glm::vec3 m_velocitySmooth;             ///< Smoothed velocity for movement
        glm::vec2 m_rotationSmooth;             ///< Smoothed rotation for mouse look

        // FOV management
        float m_targetFOV;                      ///< Target field of view
        float m_currentFOV;                     ///< Current field of view (may be transitioning)

        // Camera shake variables
        float m_shakeIntensity;                 ///< Current shake intensity
        float m_shakeDuration;                  ///< Remaining shake duration
        float m_shakeTimer;                     ///< Shake timer for tracking duration
        glm::vec3 m_shakeOffset;                ///< Current shake offset applied to camera

        // Mouse tracking variables
        bool m_firstMouse;                      ///< Flag for first mouse movement
        float m_lastMouseX;                     ///< Last mouse X position
        float m_lastMouseY;                     ///< Last mouse Y position

        // ========== Private Helper Methods ==========

        /**
         * @brief Update camera for first-person mode
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateFirstPerson(float deltaTime);

        /**
         * @brief Update camera for third-person mode
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateThirdPerson(float deltaTime);

        /**
         * @brief Update camera for spectator mode
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateSpectator(float deltaTime);

        /**
         * @brief Update camera for fixed mode
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateFixed(float deltaTime);

        /**
         * @brief Update field of view transitions
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateFOV(float deltaTime);

        /**
         * @brief Update camera shake effects
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateShake(float deltaTime);

        /**
         * @brief Apply smoothing to movement and rotation
         * @param deltaTime Time elapsed since last frame
         */
        void ApplySmoothing(float deltaTime);

        /**
         * @brief Calculate camera position for third-person mode
         * @return Calculated camera position
         */
        glm::vec3 CalculateThirdPersonPosition() const;

        /**
         * @brief Handle mouse look input
         * @param deltaTime Time elapsed since last frame
         */
        void HandleMouseLook(float deltaTime);

        // ========== Input Key Mappings ==========
        // These can be overridden in derived classes for custom key bindings

        /**
         * @brief Get the key code for forward movement
         * @return GLFW key code for moving forward
         */
        int GetMoveForwardKey() const { return GLFW_KEY_W; }

        /**
         * @brief Get the key code for backward movement
         * @return GLFW key code for moving backward
         */
        int GetMoveBackwardKey() const { return GLFW_KEY_S; }

        /**
         * @brief Get the key code for left movement
         * @return GLFW key code for moving left
         */
        int GetMoveLeftKey() const { return GLFW_KEY_A; }

        /**
         * @brief Get the key code for right movement
         * @return GLFW key code for moving right
         */
        int GetMoveRightKey() const { return GLFW_KEY_D; }

        /**
         * @brief Get the key code for upward movement
         * @return GLFW key code for moving up
         */
        int GetMoveUpKey() const { return GLFW_KEY_SPACE; }

        /**
         * @brief Get the key code for downward movement
         * @return GLFW key code for moving down
         */
        int GetMoveDownKey() const { return GLFW_KEY_LEFT_CONTROL; }

        /**
         * @brief Get the key code for sprinting
         * @return GLFW key code for sprint
         */
        int GetSprintKey() const { return GLFW_KEY_LEFT_SHIFT; }

        /**
         * @brief Get the mouse button for aiming
         * @return GLFW mouse button code for aiming
         */
        int GetAimKey() const { return GLFW_MOUSE_BUTTON_RIGHT; }
    };

} // namespace agl

#endif // CAMERA_CONTROLLER_H
