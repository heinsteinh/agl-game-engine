#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace agl {

    enum class CameraType {
        Perspective,
        Orthographic
    };

    enum class CameraMovement {
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down
    };

    class Camera {
    public:
        // Camera attributes
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        // Euler angles
        float Yaw;
        float Pitch;
        float Roll;

        // Camera options
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;

        // Perspective projection parameters
        float NearPlane;
        float FarPlane;
        float AspectRatio;

        // Orthographic projection parameters
        float OrthoLeft;
        float OrthoRight;
        float OrthoBottom;
        float OrthoTop;

        // Camera type
        CameraType Type;

        // Default camera values
        static constexpr float YAW = -90.0f;
        static constexpr float PITCH = 0.0f;
        static constexpr float ROLL = 0.0f;
        static constexpr float SPEED = 2.5f;
        static constexpr float SENSITIVITY = 0.1f;
        static constexpr float ZOOM = 45.0f;
        static constexpr float NEAR_PLANE = 0.1f;
        static constexpr float FAR_PLANE = 1000.0f;

    public:
        // Constructor with vectors
        Camera(
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = YAW,
            float pitch = PITCH,
            float roll = ROLL
        );

        // Constructor with scalar values
        Camera(
            float posX, float posY, float posZ,
            float upX, float upY, float upZ,
            float yaw = YAW,
            float pitch = PITCH,
            float roll = ROLL
        );

        // Destructor
        ~Camera() = default;

        // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
        glm::mat4 GetViewMatrix() const;

        // Returns the projection matrix
        glm::mat4 GetProjectionMatrix() const;

        // Returns the combined view-projection matrix
        glm::mat4 GetViewProjectionMatrix() const;

        // Camera movement
        void ProcessKeyboard(CameraMovement direction, float deltaTime);
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
        void ProcessMouseScroll(float yoffset);

        // Camera transformations
        void SetPosition(const glm::vec3& position);
        void SetRotation(float yaw, float pitch, float roll = 0.0f);
        void LookAt(const glm::vec3& target);

        // Projection settings
        void SetPerspective(float fov, float aspectRatio, float nearPlane = NEAR_PLANE, float farPlane = FAR_PLANE);
        void SetOrthographic(float left, float right, float bottom, float top, float nearPlane = -1.0f, float farPlane = 1.0f);

        // Getters
        glm::vec3 GetPosition() const { return Position; }
        glm::vec3 GetFront() const { return Front; }
        glm::vec3 GetUp() const { return Up; }
        glm::vec3 GetRight() const { return Right; }
        
        float GetYaw() const { return Yaw; }
        float GetPitch() const { return Pitch; }
        float GetRoll() const { return Roll; }
        
        float GetZoom() const { return Zoom; }
        float GetAspectRatio() const { return AspectRatio; }

        // Utility functions
        void Reset();
        void UpdateAspectRatio(float aspectRatio);

        // Shooter game specific functions
        glm::vec3 GetScreenToWorldRay(float screenX, float screenY, int screenWidth, int screenHeight) const;
        bool IsInView(const glm::vec3& point, float radius = 0.0f) const;

    private:
        // Calculates the front vector from the Camera's (updated) Euler Angles
        void UpdateCameraVectors();
    };

} // namespace agl

#endif // CAMERA_H
