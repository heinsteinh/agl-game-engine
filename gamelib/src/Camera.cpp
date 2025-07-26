#include "Camera.h"
#include "Logger.h"
#include <algorithm>

namespace agl {

    // Constructor with vectors
    Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float roll)
        : Position(position)
        , WorldUp(up)
        , Yaw(yaw)
        , Pitch(pitch)
        , Roll(roll)
        , Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , MovementSpeed(SPEED)
        , MouseSensitivity(SENSITIVITY)
        , Zoom(ZOOM)
        , NearPlane(NEAR_PLANE)
        , FarPlane(FAR_PLANE)
        , AspectRatio(16.0f / 9.0f)
        , OrthoLeft(-10.0f)
        , OrthoRight(10.0f)
        , OrthoBottom(-10.0f)
        , OrthoTop(10.0f)
        , Type(CameraType::Perspective)
    {
        UpdateCameraVectors();
        AGL_CORE_TRACE("Camera created at position ({:.2f}, {:.2f}, {:.2f})", Position.x, Position.y, Position.z);
    }

    // Constructor with scalar values
    Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float roll)
        : Camera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch, roll)
    {
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Returns the projection matrix
    glm::mat4 Camera::GetProjectionMatrix() const {
        if (Type == CameraType::Perspective) {
            return glm::perspective(glm::radians(Zoom), AspectRatio, NearPlane, FarPlane);
        } else {
            return glm::ortho(OrthoLeft, OrthoRight, OrthoBottom, OrthoTop, NearPlane, FarPlane);
        }
    }

    // Returns the combined view-projection matrix
    glm::mat4 Camera::GetViewProjectionMatrix() const {
        return GetProjectionMatrix() * GetViewMatrix();
    }

    // Processes input received from any keyboard-like input system
    void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        
        switch (direction) {
            case CameraMovement::Forward:
                Position += Front * velocity;
                break;
            case CameraMovement::Backward:
                Position -= Front * velocity;
                break;
            case CameraMovement::Left:
                Position -= Right * velocity;
                break;
            case CameraMovement::Right:
                Position += Right * velocity;
                break;
            case CameraMovement::Up:
                Position += WorldUp * velocity;
                break;
            case CameraMovement::Down:
                Position -= WorldUp * velocity;
                break;
        }

        AGL_CORE_TRACE("Camera moved to position ({:.2f}, {:.2f}, {:.2f})", Position.x, Position.y, Position.z);
    }

    // Processes input received from a mouse input system
    void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            Pitch = std::clamp(Pitch, -89.0f, 89.0f);
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        UpdateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event
    void Camera::ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        Zoom = std::clamp(Zoom, 1.0f, 120.0f);
        AGL_CORE_TRACE("Camera zoom changed to {:.1f}", Zoom);
    }

    // Set camera position
    void Camera::SetPosition(const glm::vec3& position) {
        Position = position;
        AGL_CORE_TRACE("Camera position set to ({:.2f}, {:.2f}, {:.2f})", Position.x, Position.y, Position.z);
    }

    // Set camera rotation
    void Camera::SetRotation(float yaw, float pitch, float roll) {
        Yaw = yaw;
        Pitch = std::clamp(pitch, -89.0f, 89.0f);
        Roll = roll;
        UpdateCameraVectors();
        AGL_CORE_TRACE("Camera rotation set to Yaw: {:.1f}, Pitch: {:.1f}, Roll: {:.1f}", Yaw, Pitch, Roll);
    }

    // Make camera look at a specific target
    void Camera::LookAt(const glm::vec3& target) {
        glm::vec3 direction = glm::normalize(target - Position);
        
        // Calculate yaw and pitch from direction vector
        Yaw = glm::degrees(atan2(direction.z, direction.x));
        Pitch = glm::degrees(asin(-direction.y));
        
        // Constrain pitch
        Pitch = std::clamp(Pitch, -89.0f, 89.0f);
        
        UpdateCameraVectors();
        AGL_CORE_TRACE("Camera looking at target ({:.2f}, {:.2f}, {:.2f})", target.x, target.y, target.z);
    }

    // Set perspective projection
    void Camera::SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
        Type = CameraType::Perspective;
        Zoom = fov;
        AspectRatio = aspectRatio;
        NearPlane = nearPlane;
        FarPlane = farPlane;
        AGL_CORE_TRACE("Camera set to perspective: FOV={:.1f}, Aspect={:.2f}, Near={:.2f}, Far={:.1f}", 
                      fov, aspectRatio, nearPlane, farPlane);
    }

    // Set orthographic projection
    void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        Type = CameraType::Orthographic;
        OrthoLeft = left;
        OrthoRight = right;
        OrthoBottom = bottom;
        OrthoTop = top;
        NearPlane = nearPlane;
        FarPlane = farPlane;
        AGL_CORE_TRACE("Camera set to orthographic: L={:.1f}, R={:.1f}, B={:.1f}, T={:.1f}", 
                      left, right, bottom, top);
    }

    // Reset camera to default state
    void Camera::Reset() {
        Position = glm::vec3(0.0f, 0.0f, 0.0f);
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = YAW;
        Pitch = PITCH;
        Roll = ROLL;
        MovementSpeed = SPEED;
        MouseSensitivity = SENSITIVITY;
        Zoom = ZOOM;
        UpdateCameraVectors();
        AGL_CORE_INFO("Camera reset to default state");
    }

    // Update aspect ratio
    void Camera::UpdateAspectRatio(float aspectRatio) {
        AspectRatio = aspectRatio;
        AGL_CORE_TRACE("Camera aspect ratio updated to {:.2f}", aspectRatio);
    }

    // Get ray from screen coordinates (useful for mouse picking in shooters)
    glm::vec3 Camera::GetScreenToWorldRay(float screenX, float screenY, int screenWidth, int screenHeight) const {
        // Normalize screen coordinates to [-1, 1]
        float x = (2.0f * screenX) / screenWidth - 1.0f;
        float y = 1.0f - (2.0f * screenY) / screenHeight;
        
        // Create ray in clip space
        glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
        
        // Transform to eye space
        glm::mat4 projInverse = glm::inverse(GetProjectionMatrix());
        glm::vec4 rayEye = projInverse * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
        
        // Transform to world space
        glm::mat4 viewInverse = glm::inverse(GetViewMatrix());
        glm::vec4 rayWorld = viewInverse * rayEye;
        
        return glm::normalize(glm::vec3(rayWorld));
    }

    // Check if a point is in the camera's view frustum
    bool Camera::IsInView(const glm::vec3& point, float radius) const {
        glm::mat4 viewProj = GetViewProjectionMatrix();
        glm::vec4 clipSpacePos = viewProj * glm::vec4(point, 1.0f);
        
        // Perspective divide
        glm::vec3 ndcPos = glm::vec3(clipSpacePos) / clipSpacePos.w;
        
        // Check if point is within the view frustum (with radius tolerance)
        float tolerance = radius / std::max(std::abs(clipSpacePos.w), 1.0f);
        
        return (ndcPos.x >= -1.0f - tolerance && ndcPos.x <= 1.0f + tolerance &&
                ndcPos.y >= -1.0f - tolerance && ndcPos.y <= 1.0f + tolerance &&
                ndcPos.z >= -1.0f - tolerance && ndcPos.z <= 1.0f + tolerance);
    }

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void Camera::UpdateCameraVectors() {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
        
        // Apply roll rotation if needed
        if (std::abs(Roll) > 0.001f) {
            glm::mat3 rollMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(Roll), Front));
            Right = rollMatrix * Right;
            Up = rollMatrix * Up;
        }
    }

} // namespace agl
