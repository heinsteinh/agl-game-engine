#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "Camera.h"
#include "input.h"
#include <memory>

namespace agl {

    enum class CameraMode {
        FirstPerson,    // Standard FPS camera
        ThirdPerson,    // Third person camera with offset
        Spectator,      // Free-flying camera
        Fixed           // Fixed position camera
    };

    struct CameraSettings {
        // Movement settings
        float movementSpeed = 5.0f;
        float sprintMultiplier = 2.0f;
        float crouchMultiplier = 0.5f;

        // Mouse settings
        float mouseSensitivity = 0.1f;
        bool invertY = false;

        // Smoothing settings
        float movementSmoothing = 0.1f;
        float rotationSmoothing = 0.1f;

        // Third person settings
        float thirdPersonDistance = 5.0f;
        float thirdPersonHeight = 2.0f;
        glm::vec3 thirdPersonOffset = glm::vec3(0.0f, 0.0f, 0.0f);

        // Constraints
        bool constrainPitch = true;
        float minPitch = -89.0f;
        float maxPitch = 89.0f;

        // Field of view settings
        float defaultFOV = 75.0f;
        float sprintFOV = 85.0f;
        float aimFOV = 50.0f;
        float fovTransitionSpeed = 5.0f;

        // Shake settings
        float shakeIntensity = 1.0f;
        float shakeDamping = 5.0f;
    };

    class CameraController {
    public:
        CameraController();
        explicit CameraController(std::shared_ptr<Camera> camera);
        ~CameraController() = default;

        // Initialize with input system
        void Initialize(Input* input);

        // Update camera based on input and delta time
        void Update(float deltaTime);

        // Camera management
        void SetCamera(std::shared_ptr<Camera> camera);
        std::shared_ptr<Camera> GetCamera() const { return m_camera; }

        // Mode management
        void SetMode(CameraMode mode);
        CameraMode GetMode() const { return m_mode; }

        // Settings
        void SetSettings(const CameraSettings& settings);
        CameraSettings& GetSettings() { return m_settings; }
        const CameraSettings& GetSettings() const { return m_settings; }

        // Position and target management (useful for third person)
        void SetTarget(const glm::vec3& target);
        void SetPosition(const glm::vec3& position);
        glm::vec3 GetTarget() const { return m_target; }

        // Shooter-specific features
        void StartAiming();
        void StopAiming();
        bool IsAiming() const { return m_isAiming; }

        void StartSprinting();
        void StopSprinting();
        bool IsSprinting() const { return m_isSprinting; }

        void StartCrouching();
        void StopCrouching();
        bool IsCrouching() const { return m_isCrouching; }

        // Camera shake effects
        void AddShake(float intensity, float duration = 0.5f);
        void ClearShake();

        // FOV control
        void SetFOV(float fov);
        float GetCurrentFOV() const { return m_currentFOV; }

        // Input handling
        void ProcessKeyboard(float deltaTime);
        void ProcessMouse(float deltaTime);
        void ProcessMouseScroll(float yoffset);

        // Reset camera to default state
        void Reset();

        // Enable/disable input processing
        void SetInputEnabled(bool enabled) { m_inputEnabled = enabled; }
        bool IsInputEnabled() const { return m_inputEnabled; }

        // Smoothing toggle
        void SetSmoothingEnabled(bool enabled) { m_smoothingEnabled = enabled; }
        bool IsSmoothingEnabled() const { return m_smoothingEnabled; }

    private:
        std::shared_ptr<Camera> m_camera;
        Input* m_input;
        CameraMode m_mode;
        CameraSettings m_settings;

        // Target for third person mode
        glm::vec3 m_target;

        // Camera states
        bool m_isAiming;
        bool m_isSprinting;
        bool m_isCrouching;
        bool m_inputEnabled;
        bool m_smoothingEnabled;

        // Smoothing variables
        glm::vec3 m_velocitySmooth;
        glm::vec2 m_rotationSmooth;

        // FOV management
        float m_targetFOV;
        float m_currentFOV;

        // Camera shake
        float m_shakeIntensity;
        float m_shakeDuration;
        float m_shakeTimer;
        glm::vec3 m_shakeOffset;

        // Mouse tracking
        bool m_firstMouse;
        float m_lastMouseX;
        float m_lastMouseY;

        // Helper methods
        void UpdateFirstPerson(float deltaTime);
        void UpdateThirdPerson(float deltaTime);
        void UpdateSpectator(float deltaTime);
        void UpdateFixed(float deltaTime);

        void UpdateFOV(float deltaTime);
        void UpdateShake(float deltaTime);
        void ApplySmoothing(float deltaTime);

        glm::vec3 CalculateThirdPersonPosition() const;
        void HandleMouseLook(float deltaTime);

        // Input key mappings (can be customized)
        int GetMoveForwardKey() const { return GLFW_KEY_W; }
        int GetMoveBackwardKey() const { return GLFW_KEY_S; }
        int GetMoveLeftKey() const { return GLFW_KEY_A; }
        int GetMoveRightKey() const { return GLFW_KEY_D; }
        int GetMoveUpKey() const { return GLFW_KEY_SPACE; }
        int GetMoveDownKey() const { return GLFW_KEY_LEFT_CONTROL; }
        int GetSprintKey() const { return GLFW_KEY_LEFT_SHIFT; }
        int GetAimKey() const { return GLFW_MOUSE_BUTTON_RIGHT; }
    };

} // namespace agl

#endif // CAMERA_CONTROLLER_H
