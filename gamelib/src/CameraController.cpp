#include "CameraController.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace agl {

    CameraController::CameraController()
        : m_camera(nullptr)
        , m_input(nullptr)
        , m_mode(CameraMode::FirstPerson)
        , m_settings()
        , m_target(0.0f, 0.0f, 0.0f)
        , m_isAiming(false)
        , m_isSprinting(false)
        , m_isCrouching(false)
        , m_inputEnabled(true)
        , m_smoothingEnabled(true)
        , m_velocitySmooth(0.0f)
        , m_rotationSmooth(0.0f)
        , m_targetFOV(75.0f)
        , m_currentFOV(75.0f)
        , m_shakeIntensity(0.0f)
        , m_shakeDuration(0.0f)
        , m_shakeTimer(0.0f)
        , m_shakeOffset(0.0f)
        , m_firstMouse(true)
        , m_lastMouseX(0.0f)
        , m_lastMouseY(0.0f)
    {
        AGL_CORE_INFO("CameraController created");
    }

    CameraController::CameraController(std::shared_ptr<Camera> camera)
        : CameraController()
    {
        SetCamera(camera);
    }

    void CameraController::Initialize(Input* input) {
        m_input = input;
        if (m_camera) {
            m_targetFOV = m_settings.defaultFOV;
            m_currentFOV = m_settings.defaultFOV;
            m_camera->SetPerspective(m_currentFOV, m_camera->GetAspectRatio());
        }
        AGL_CORE_INFO("CameraController initialized with input system");
    }

    void CameraController::Update(float deltaTime) {
        if (!m_camera || !m_input || !m_inputEnabled) {
            return;
        }

        // Process input
        ProcessKeyboard(deltaTime);
        ProcessMouse(deltaTime);

        // Update camera based on mode
        switch (m_mode) {
            case CameraMode::FirstPerson:
                UpdateFirstPerson(deltaTime);
                break;
            case CameraMode::ThirdPerson:
                UpdateThirdPerson(deltaTime);
                break;
            case CameraMode::Spectator:
                UpdateSpectator(deltaTime);
                break;
            case CameraMode::Fixed:
                UpdateFixed(deltaTime);
                break;
        }

        // Update camera effects
        UpdateFOV(deltaTime);
        UpdateShake(deltaTime);
        
        if (m_smoothingEnabled) {
            ApplySmoothing(deltaTime);
        }
    }

    void CameraController::SetCamera(std::shared_ptr<Camera> camera) {
        m_camera = camera;
        if (m_camera) {
            m_targetFOV = m_settings.defaultFOV;
            m_currentFOV = m_settings.defaultFOV;
            AGL_CORE_INFO("Camera attached to controller");
        }
    }

    void CameraController::SetMode(CameraMode mode) {
        if (m_mode != mode) {
            m_mode = mode;
            AGL_CORE_INFO("Camera mode changed to: {}", static_cast<int>(mode));
            
            // Reset smooth values when changing modes
            m_velocitySmooth = glm::vec3(0.0f);
            m_rotationSmooth = glm::vec2(0.0f);
        }
    }

    void CameraController::SetSettings(const CameraSettings& settings) {
        m_settings = settings;
        if (m_camera) {
            m_camera->MovementSpeed = settings.movementSpeed;
            m_camera->MouseSensitivity = settings.mouseSensitivity;
        }
        AGL_CORE_TRACE("Camera settings updated");
    }

    void CameraController::SetTarget(const glm::vec3& target) {
        m_target = target;
        AGL_CORE_TRACE("Camera target set to ({:.2f}, {:.2f}, {:.2f})", target.x, target.y, target.z);
    }

    void CameraController::SetPosition(const glm::vec3& position) {
        if (m_camera) {
            m_camera->SetPosition(position);
        }
    }

    void CameraController::StartAiming() {
        if (!m_isAiming) {
            m_isAiming = true;
            m_targetFOV = m_settings.aimFOV;
            AGL_CORE_TRACE("Started aiming - FOV target: {:.1f}", m_targetFOV);
        }
    }

    void CameraController::StopAiming() {
        if (m_isAiming) {
            m_isAiming = false;
            m_targetFOV = m_isSprinting ? m_settings.sprintFOV : m_settings.defaultFOV;
            AGL_CORE_TRACE("Stopped aiming - FOV target: {:.1f}", m_targetFOV);
        }
    }

    void CameraController::StartSprinting() {
        if (!m_isSprinting) {
            m_isSprinting = true;
            if (!m_isAiming) {
                m_targetFOV = m_settings.sprintFOV;
            }
            AGL_CORE_TRACE("Started sprinting");
        }
    }

    void CameraController::StopSprinting() {
        if (m_isSprinting) {
            m_isSprinting = false;
            if (!m_isAiming) {
                m_targetFOV = m_settings.defaultFOV;
            }
            AGL_CORE_TRACE("Stopped sprinting");
        }
    }

    void CameraController::StartCrouching() {
        if (!m_isCrouching) {
            m_isCrouching = true;
            AGL_CORE_TRACE("Started crouching");
        }
    }

    void CameraController::StopCrouching() {
        if (m_isCrouching) {
            m_isCrouching = false;
            AGL_CORE_TRACE("Stopped crouching");
        }
    }

    void CameraController::AddShake(float intensity, float duration) {
        m_shakeIntensity = std::max(m_shakeIntensity, intensity * m_settings.shakeIntensity);
        m_shakeDuration = std::max(m_shakeDuration, duration);
        m_shakeTimer = m_shakeDuration;
        AGL_CORE_TRACE("Camera shake added: intensity={:.2f}, duration={:.2f}", intensity, duration);
    }

    void CameraController::ClearShake() {
        m_shakeIntensity = 0.0f;
        m_shakeDuration = 0.0f;
        m_shakeTimer = 0.0f;
        m_shakeOffset = glm::vec3(0.0f);
        AGL_CORE_TRACE("Camera shake cleared");
    }

    void CameraController::SetFOV(float fov) {
        m_targetFOV = fov;
        AGL_CORE_TRACE("Target FOV set to {:.1f}", fov);
    }

    void CameraController::ProcessKeyboard(float deltaTime) {
        if (!m_input) return;

        // Handle sprint
        if (m_input->IsKeyHeld(GetSprintKey())) {
            if (!m_isSprinting) StartSprinting();
        } else {
            if (m_isSprinting) StopSprinting();
        }

        // Handle crouch
        if (m_input->IsKeyHeld(GetMoveDownKey())) {
            if (!m_isCrouching) StartCrouching();
        } else {
            if (m_isCrouching) StopCrouching();
        }

        // Movement processing
        glm::vec3 movement(0.0f);
        
        if (m_input->IsKeyHeld(GetMoveForwardKey())) {
            movement += m_camera->GetFront();
        }
        if (m_input->IsKeyHeld(GetMoveBackwardKey())) {
            movement -= m_camera->GetFront();
        }
        if (m_input->IsKeyHeld(GetMoveLeftKey())) {
            movement -= m_camera->GetRight();
        }
        if (m_input->IsKeyHeld(GetMoveRightKey())) {
            movement += m_camera->GetRight();
        }
        if (m_input->IsKeyHeld(GetMoveUpKey())) {
            movement += m_camera->GetUp();
        }

        // Normalize movement to prevent faster diagonal movement
        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement);
            
            // Apply speed modifiers
            float speed = m_settings.movementSpeed;
            if (m_isSprinting) speed *= m_settings.sprintMultiplier;
            if (m_isCrouching) speed *= m_settings.crouchMultiplier;
            
            movement *= speed * deltaTime;
            
            if (m_smoothingEnabled) {
                // Apply movement smoothing
                m_velocitySmooth = glm::mix(m_velocitySmooth, movement, m_settings.movementSmoothing);
                m_camera->SetPosition(m_camera->GetPosition() + m_velocitySmooth);
            } else {
                m_camera->SetPosition(m_camera->GetPosition() + movement);
            }
        } else if (m_smoothingEnabled) {
            // Dampen velocity when no input
            m_velocitySmooth = glm::mix(m_velocitySmooth, glm::vec3(0.0f), m_settings.movementSmoothing * 2.0f);
            m_camera->SetPosition(m_camera->GetPosition() + m_velocitySmooth);
        }
    }

    void CameraController::ProcessMouse(float deltaTime) {
        if (!m_input) return;

        // Handle aim
        if (m_input->IsMouseButtonHeld(agl::MouseButton::Right)) {
            if (!m_isAiming) StartAiming();
        } else {
            if (m_isAiming) StopAiming();
        }

        // Handle mouse look
        HandleMouseLook(deltaTime);
    }

    void CameraController::ProcessMouseScroll(float yoffset) {
        if (m_camera && m_inputEnabled) {
            m_camera->ProcessMouseScroll(yoffset);
        }
    }

    void CameraController::Reset() {
        if (m_camera) {
            m_camera->Reset();
        }
        
        m_isAiming = false;
        m_isSprinting = false;
        m_isCrouching = false;
        m_velocitySmooth = glm::vec3(0.0f);
        m_rotationSmooth = glm::vec2(0.0f);
        m_targetFOV = m_settings.defaultFOV;
        m_currentFOV = m_settings.defaultFOV;
        ClearShake();
        m_firstMouse = true;
        
        AGL_CORE_INFO("Camera controller reset");
    }

    void CameraController::UpdateFirstPerson(float deltaTime) {
        // First person is handled by direct camera updates in ProcessKeyboard/Mouse
        // No additional processing needed for basic first person
    }

    void CameraController::UpdateThirdPerson(float deltaTime) {
        if (!m_camera) return;
        
        // Calculate ideal third person position
        glm::vec3 idealPosition = CalculateThirdPersonPosition();
        
        // Smoothly move camera to ideal position
        if (m_smoothingEnabled) {
            glm::vec3 currentPos = m_camera->GetPosition();
            glm::vec3 newPos = glm::mix(currentPos, idealPosition, m_settings.movementSmoothing * 2.0f);
            m_camera->SetPosition(newPos);
        } else {
            m_camera->SetPosition(idealPosition);
        }
        
        // Always look at target in third person
        m_camera->LookAt(m_target);
    }

    void CameraController::UpdateSpectator(float deltaTime) {
        // Spectator mode allows free movement and rotation
        // Movement is handled in ProcessKeyboard, rotation in ProcessMouse
        // No additional constraints
    }

    void CameraController::UpdateFixed(float deltaTime) {
        // Fixed mode might still want to track a target
        if (glm::length(m_target) > 0.0f) {
            m_camera->LookAt(m_target);
        }
    }

    void CameraController::UpdateFOV(float deltaTime) {
        if (!m_camera) return;
        
        // Smoothly interpolate FOV
        if (std::abs(m_currentFOV - m_targetFOV) > 0.1f) {
            m_currentFOV = glm::mix(m_currentFOV, m_targetFOV, m_settings.fovTransitionSpeed * deltaTime);
            m_camera->SetPerspective(m_currentFOV, m_camera->GetAspectRatio());
        }
    }

    void CameraController::UpdateShake(float deltaTime) {
        if (m_shakeTimer > 0.0f) {
            m_shakeTimer -= deltaTime;
            
            // Generate random shake offset
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
            
            float intensity = m_shakeIntensity * (m_shakeTimer / m_shakeDuration);
            m_shakeOffset.x = dis(gen) * intensity;
            m_shakeOffset.y = dis(gen) * intensity;
            m_shakeOffset.z = dis(gen) * intensity * 0.5f; // Less Z shake
            
            // Apply shake to camera position
            if (m_camera) {
                glm::vec3 basePos = m_camera->GetPosition();
                m_camera->SetPosition(basePos + m_shakeOffset);
            }
            
            // Dampen shake over time
            m_shakeIntensity = std::max(0.0f, m_shakeIntensity - m_settings.shakeDamping * deltaTime);
        } else {
            // Clear shake when timer expires
            if (m_shakeIntensity > 0.0f) {
                ClearShake();
            }
        }
    }

    void CameraController::ApplySmoothing(float deltaTime) {
        // Additional smoothing is handled in individual update methods
        // This could be extended for more complex smoothing behaviors
    }

    glm::vec3 CameraController::CalculateThirdPersonPosition() const {
        // Calculate position behind and above the target
        glm::vec3 direction = glm::normalize(m_camera->GetFront());
        glm::vec3 position = m_target;
        position -= direction * m_settings.thirdPersonDistance;
        position.y += m_settings.thirdPersonHeight;
        position += m_settings.thirdPersonOffset;
        return position;
    }

    void CameraController::HandleMouseLook(float deltaTime) {
        if (!m_input || !m_camera) return;
        
        double mouseX, mouseY;
        m_input->GetMousePosition(mouseX, mouseY);
        
        if (m_firstMouse) {
            m_lastMouseX = static_cast<float>(mouseX);
            m_lastMouseY = static_cast<float>(mouseY);
            m_firstMouse = false;
            return;
        }
        
        float xoffset = static_cast<float>(mouseX) - m_lastMouseX;
        float yoffset = m_lastMouseY - static_cast<float>(mouseY); // Reversed since y-coordinates go from bottom to top
        
        m_lastMouseX = static_cast<float>(mouseX);
        m_lastMouseY = static_cast<float>(mouseY);
        
        // Apply sensitivity
        xoffset *= m_settings.mouseSensitivity;
        yoffset *= m_settings.mouseSensitivity;
        
        // Invert Y if enabled
        if (m_settings.invertY) {
            yoffset = -yoffset;
        }
        
        if (m_smoothingEnabled) {
            // Apply rotation smoothing
            glm::vec2 targetRotation(xoffset, yoffset);
            m_rotationSmooth = glm::mix(m_rotationSmooth, targetRotation, m_settings.rotationSmoothing);
            m_camera->ProcessMouseMovement(m_rotationSmooth.x, m_rotationSmooth.y, m_settings.constrainPitch);
        } else {
            m_camera->ProcessMouseMovement(xoffset, yoffset, m_settings.constrainPitch);
        }
    }

} // namespace agl
