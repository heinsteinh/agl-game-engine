/**
 * @file Camera.h
 * @brief Core 3D camera implementation for the AGL Game Engine
 * @author AGL Team
 * @version 1.0
 * @date 2025
 *
 * This file contains the Camera class which provides comprehensive 3D camera functionality
 * including perspective and orthographic projections, Euler angle rotation, mouse and keyboard
 * input processing, and advanced features for game development such as screen-to-world ray
 * casting and view frustum culling.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace agl {

/**
 * @brief Enumeration of camera projection types
 *
 * Defines the type of projection used by the camera for rendering.
 */
enum class CameraType {
    Perspective, ///< Perspective projection (3D depth, objects further away appear smaller)
    Orthographic ///< Orthographic projection (2D-like, no perspective distortion)
};

/**
 * @brief Enumeration of camera movement directions
 *
 * Defines the possible movement directions for camera input processing.
 * Used with ProcessKeyboard() to move the camera in world space.
 */
enum class CameraMovement {
    Forward,  ///< Move camera forward along its front vector
    Backward, ///< Move camera backward (opposite of front vector)
    Left,     ///< Move camera left (strafe left using right vector)
    Right,    ///< Move camera right (strafe right using right vector)
    Up,       ///< Move camera up along world up vector
    Down      ///< Move camera down (opposite of world up vector)
};

/**
 * @brief 3D Camera class for the AGL Game Engine
 *
 * The Camera class provides a comprehensive 3D camera implementation with support for:
 * - Perspective and orthographic projections
 * - Euler angle rotation (yaw, pitch, roll)
 * - Keyboard and mouse input processing
 * - Smooth movement and look controls
 * - Screen-to-world ray casting for mouse picking
 * - View frustum culling for performance optimization
 * - Shooter game specific features
 *
 * The camera uses a right-handed coordinate system with Y-up orientation.
 *
 * @example Basic Camera Usage
 * @code
 * // Create a camera at position (0, 2, 5) looking forward
 * auto camera = std::make_shared<agl::Camera>(
 *     glm::vec3(0.0f, 2.0f, 5.0f),  // Position
 *     glm::vec3(0.0f, 1.0f, 0.0f),  // Up vector
 *     -90.0f,                        // Yaw (facing negative Z)
 *     0.0f                          // Pitch (level)
 * );
 *
 * // Set perspective projection
 * camera->SetPerspective(75.0f, 16.0f/9.0f);
 *
 * // In game loop
 * camera->ProcessKeyboard(agl::CameraMovement::Forward, deltaTime);
 * glm::mat4 viewProjection = camera->GetViewProjectionMatrix();
 * @endcode
 *
 * @note This class is designed to be used with a CameraController for high-level
 *       game functionality, though it can be used standalone for custom camera systems.
 *
 * @see CameraController for high-level camera control
 * @see CameraMovement for movement direction options
 * @see CameraType for projection type options
 */
class Camera {
public:
    // ========== Public Attributes ==========

    /**
     * @brief Current position of the camera in world space
     *
     * This vector represents the camera's location in 3D world coordinates.
     * Can be modified directly or through SetPosition().
     */
    glm::vec3 Position;

    /**
     * @brief Forward direction vector of the camera
     *
     * Unit vector pointing in the direction the camera is facing.
     * Automatically calculated from Euler angles in UpdateCameraVectors().
     */
    glm::vec3 Front;

    /**
     * @brief Up direction vector of the camera
     *
     * Unit vector pointing upward relative to the camera's orientation.
     * Automatically calculated from Front and Right vectors.
     */
    glm::vec3 Up;

    /**
     * @brief Right direction vector of the camera
     *
     * Unit vector pointing to the right of the camera's orientation.
     * Automatically calculated from Front and WorldUp vectors.
     */
    glm::vec3 Right;

    /**
     * @brief World up vector (typically Y-up)
     *
     * Reference vector for world's up direction, used to calculate
     * the camera's relative up and right vectors. Usually (0, 1, 0).
     */
    glm::vec3 WorldUp;

    // ========== Euler Angles ==========

    /**
     * @brief Yaw angle in degrees
     *
     * Rotation around the world up axis (Y-axis by default).
     * - 0° = facing positive X
     * - -90° = facing negative Z (forward in OpenGL)
     * - 90° = facing positive Z
     * - 180° = facing negative X
     */
    float Yaw;

    /**
     * @brief Pitch angle in degrees
     *
     * Rotation around the camera's right axis.
     * - 0° = looking straight ahead
     * - 90° = looking straight up
     * - -90° = looking straight down
     *
     * @note Usually constrained to [-89°, 89°] to prevent gimbal lock
     */
    float Pitch;

    /**
     * @brief Roll angle in degrees
     *
     * Rotation around the camera's front axis (barrel roll).
     * - 0° = camera upright
     * - 90° = camera tilted 90° clockwise
     * - -90° = camera tilted 90° counter-clockwise
     *
     * @note Roll is rarely used in most game cameras
     */
    float Roll;

    // ========== Camera Options ==========

    /**
     * @brief Movement speed in units per second
     *
     * Controls how fast the camera moves when ProcessKeyboard() is called.
     * Higher values result in faster movement.
     *
     * @note This is used as a base speed; actual movement can be modified
     *       by delta time and other factors in derived controllers.
     */
    float MovementSpeed;

    /**
     * @brief Mouse sensitivity multiplier
     *
     * Controls how sensitive the camera is to mouse movement.
     * Lower values = less sensitive (smoother), higher values = more sensitive.
     * Typical range: 0.05 to 0.3
     */
    float MouseSensitivity;

    /**
     * @brief Field of view in degrees (for perspective projection)
     *
     * Controls the camera's field of view angle. Also used as zoom level
     * in some contexts. Typical values:
     * - 45° = narrow FOV (zoomed in)
     * - 75° = standard FOV
     * - 90° = wide FOV
     *
     * @note Only applies to perspective projection
     */
    float Zoom;

    // ========== Projection Parameters ==========

    /**
     * @brief Near clipping plane distance
     *
     * Minimum distance from camera for objects to be rendered.
     * Objects closer than this distance are clipped.
     * Should be as large as possible while still showing nearby objects.
     */
    float NearPlane;

    /**
     * @brief Far clipping plane distance
     *
     * Maximum distance from camera for objects to be rendered.
     * Objects farther than this distance are clipped.
     * Should be as small as possible while still showing distant objects.
     */
    float FarPlane;

    /**
     * @brief Aspect ratio (width/height) of the viewport
     *
     * Used for perspective projection to maintain correct proportions.
     * Should be updated whenever the window/viewport size changes.
     * Example: 1920/1080 = 1.777 (16:9 aspect ratio)
     */
    float AspectRatio;

    // ========== Orthographic Parameters ==========

    /**
     * @brief Left boundary for orthographic projection
     * @see SetOrthographic()
     */
    float OrthoLeft;
    /**
     * @brief Right boundary for orthographic projection
     * @see SetOrthographic()
     */
    float OrthoRight;

    /**
     * @brief Bottom boundary for orthographic projection
     * @see SetOrthographic()
     */
    float OrthoBottom;

    /**
     * @brief Top boundary for orthographic projection
     * @see SetOrthographic()
     */
    float OrthoTop;

    /**
     * @brief Current camera projection type
     * @see CameraType
     */
    CameraType Type;

    // ========== Default Values ==========

    static constexpr float YAW = -90.0f;        ///< Default yaw angle (facing negative Z)
    static constexpr float PITCH = 0.0f;        ///< Default pitch angle (looking straight ahead)
    static constexpr float ROLL = 0.0f;         ///< Default roll angle (upright)
    static constexpr float SPEED = 2.5f;        ///< Default movement speed
    static constexpr float SENSITIVITY = 0.1f;  ///< Default mouse sensitivity
    static constexpr float ZOOM = 45.0f;        ///< Default field of view
    static constexpr float NEAR_PLANE = 0.1f;   ///< Default near clipping plane
    static constexpr float FAR_PLANE = 1000.0f; ///< Default far clipping plane

public:
    // ========== Constructors ==========

    /**
     * @brief Construct a camera with vector parameters
     *
     * Creates a camera at the specified position with the given orientation.
     * The camera will use perspective projection by default.
     *
     * @param position Initial world position of the camera
     * @param up World up vector (usually (0, 1, 0))
     * @param yaw Initial yaw angle in degrees
     * @param pitch Initial pitch angle in degrees
     * @param roll Initial roll angle in degrees
     *
     * @note The front vector is automatically calculated from the Euler angles
     *
     * @example
     * @code
     * // Create camera 5 units back from origin, looking forward
     * agl::Camera camera(glm::vec3(0, 0, 5), glm::vec3(0, 1, 0), -90.0f, 0.0f);
     * @endcode
     */
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH, float roll = ROLL);

    /**
     * @brief Construct a camera with scalar parameters
     *
     * Alternative constructor using individual float values instead of vectors.
     * Useful when position and up vector components are calculated separately.
     *
     * @param posX X component of camera position
     * @param posY Y component of camera position
     * @param posZ Z component of camera position
     * @param upX X component of world up vector
     * @param upY Y component of world up vector
     * @param upZ Z component of world up vector
     * @param yaw Initial yaw angle in degrees
     * @param pitch Initial pitch angle in degrees
     * @param roll Initial roll angle in degrees
     *
     * @example
     * @code
     * // Create camera at (10, 5, 0) with standard Y-up
     * agl::Camera camera(10.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.0f);
     * @endcode
     */
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw = YAW, float pitch = PITCH,
           float roll = ROLL);

    /**
     * @brief Destructor
     *
     * Default destructor. Camera class uses only stack-allocated data
     * so no special cleanup is required.
     */
    ~Camera() = default;

    // ========== Matrix Functions ==========

    /**
     * @brief Get the view matrix for rendering
     *
     * Returns the view matrix calculated using the camera's current position
     * and orientation. This matrix transforms world coordinates to camera/view space.
     *
     * @return 4x4 view matrix
     *
     * @note The view matrix is calculated using glm::lookAt internally
     *
     * @example
     * @code
     * glm::mat4 view = camera.GetViewMatrix();
     * shader.SetUniform("u_ViewMatrix", view);
     * @endcode
     */
    glm::mat4 GetViewMatrix() const;

    /**
     * @brief Get the projection matrix for rendering
     *
     * Returns the current projection matrix (perspective or orthographic)
     * based on the camera's Type and projection parameters.
     *
     * @return 4x4 projection matrix
     *
     * @see SetPerspective()
     * @see SetOrthographic()
     *
     * @example
     * @code
     * glm::mat4 projection = camera.GetProjectionMatrix();
     * shader.SetUniform("u_ProjectionMatrix", projection);
     * @endcode
     */
    glm::mat4 GetProjectionMatrix() const;

    /**
     * @brief Get the combined view-projection matrix
     *
     * Returns the result of projection * view matrices. This is the most
     * commonly used matrix for rendering as it transforms directly from
     * world space to clip space.
     *
     * @return 4x4 view-projection matrix
     *
     * @note This is equivalent to GetProjectionMatrix() * GetViewMatrix()
     *       but may be optimized in the implementation
     *
     * @example
     * @code
     * glm::mat4 viewProj = camera.GetViewProjectionMatrix();
     * glm::mat4 mvp = viewProj * modelMatrix;
     * shader.SetUniform("u_MVP", mvp);
     * @endcode
     */
    glm::mat4 GetViewProjectionMatrix() const;

    // ========== Input Processing ==========

    /**
     * @brief Process keyboard input for camera movement
     *
     * Moves the camera in the specified direction based on its current
     * orientation. The movement is scaled by MovementSpeed and deltaTime.
     *
     * @param direction Direction to move the camera
     * @param deltaTime Time elapsed since last frame (for frame-rate independent movement)
     *
     * @note Movement is relative to camera orientation:
     *       - Forward/Backward: along the Front vector
     *       - Left/Right: along the Right vector (strafing)
     *       - Up/Down: along the WorldUp vector
     *
     * @example
     * @code
     * // In game loop with WASD controls
     * if (input.IsKeyPressed(GLFW_KEY_W))
     *     camera.ProcessKeyboard(agl::CameraMovement::Forward, deltaTime);
     * if (input.IsKeyPressed(GLFW_KEY_S))
     *     camera.ProcessKeyboard(agl::CameraMovement::Backward, deltaTime);
     * @endcode
     */
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    /**
     * @brief Process mouse movement for camera rotation
     *
     * Updates the camera's yaw and pitch based on mouse movement offsets.
     * This creates the standard "mouse look" behavior found in most 3D games.
     *
     * @param xoffset Horizontal mouse movement (positive = right)
     * @param yoffset Vertical mouse movement (positive = up)
     * @param constrainPitch Whether to constrain pitch to prevent over-rotation
     *
     * @note When constrainPitch is true, pitch is clamped to [-89°, 89°] to
     *       prevent the camera from flipping over (gimbal lock)
     *
     * @example
     * @code
     * // In mouse callback
     * double mouseX, mouseY;
     * glfwGetCursorPos(window, &mouseX, &mouseY);
     *
     * float xOffset = mouseX - lastMouseX;
     * float yOffset = lastMouseY - mouseY; // Reversed since y-coordinates go from bottom to top
     *
     * camera.ProcessMouseMovement(xOffset, yOffset);
     * @endcode
     */
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    /**
     * @brief Process mouse scroll wheel for zoom control
     *
     * Adjusts the camera's zoom level (field of view for perspective cameras)
     * based on scroll wheel input. Commonly used for zooming in/out.
     *
     * @param yoffset Scroll wheel offset (positive = zoom in, negative = zoom out)
     *
     * @note For perspective cameras, this modifies the Zoom field which represents FOV.
     *       Zoom is constrained to reasonable values (typically 1° to 75°).
     *
     * @example
     * @code
     * // In scroll callback
     * void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
     *     camera.ProcessMouseScroll(static_cast<float>(yoffset));
     * }
     * @endcode
     */
    void ProcessMouseScroll(float yoffset);

    // ========== Camera Transformations ==========

    /**
     * @brief Set the camera's world position
     *
     * Directly sets the camera's position in world coordinates.
     * This does not affect the camera's orientation.
     *
     * @param position New world position for the camera
     *
     * @example
     * @code
     * // Place camera 10 units above the origin
     * camera.SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
     * @endcode
     */
    void SetPosition(const glm::vec3 &position);

    /**
     * @brief Set the camera's rotation using Euler angles
     *
     * Sets the camera's orientation using yaw, pitch, and roll angles.
     * The camera's direction vectors are automatically updated.
     *
     * @param yaw Rotation around world up axis (degrees)
     * @param pitch Rotation around camera right axis (degrees)
     * @param roll Rotation around camera front axis (degrees)
     *
     * @note Angles are in degrees, not radians
     *
     * @example
     * @code
     * // Look 45 degrees to the right and 30 degrees down
     * camera.SetRotation(-45.0f, -30.0f);
     * @endcode
     */
    void SetRotation(float yaw, float pitch, float roll = 0.0f);

    /**
     * @brief Make the camera look at a specific target point
     *
     * Automatically calculates and sets the camera's rotation to look
     * directly at the specified target position. The camera's position
     * is not changed.
     *
     * @param target World position to look at
     *
     * @note This calculates the required yaw and pitch angles but sets roll to 0
     *
     * @example
     * @code
     * // Make camera look at the origin
     * camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
     *
     * // Look at a moving player
     * camera.LookAt(player.GetPosition());
     * @endcode
     */
    void LookAt(const glm::vec3 &target);

    // ========== Projection Settings ==========

    /**
     * @brief Set perspective projection parameters
     *
     * Configures the camera to use perspective projection with the specified
     * field of view, aspect ratio, and clipping planes. This is the most
     * common projection type for 3D games.
     *
     * @param fov Field of view angle in degrees (typically 45-90)
     * @param aspectRatio Viewport width divided by height
     * @param nearPlane Distance to near clipping plane (should be > 0)
     * @param farPlane Distance to far clipping plane (should be > nearPlane)
     *
     * @note Sets the camera Type to CameraType::Perspective
     *
     * @example
     * @code
     * // Standard 75 degree FOV with 16:9 aspect ratio
     * camera.SetPerspective(75.0f, 1920.0f/1080.0f, 0.1f, 1000.0f);
     * @endcode
     */
    void SetPerspective(float fov, float aspectRatio, float nearPlane = NEAR_PLANE, float farPlane = FAR_PLANE);

    /**
     * @brief Set orthographic projection parameters
     *
     * Configures the camera to use orthographic projection with the specified
     * boundaries. Objects maintain their size regardless of distance from camera.
     * Commonly used for 2D games, UI overlays, or technical drawings.
     *
     * @param left Left boundary of the view volume
     * @param right Right boundary of the view volume
     * @param bottom Bottom boundary of the view volume
     * @param top Top boundary of the view volume
     * @param nearPlane Distance to near clipping plane
     * @param farPlane Distance to far clipping plane
     *
     * @note Sets the camera Type to CameraType::Orthographic
     *
     * @example
     * @code
     * // Create a 20x20 unit orthographic view centered at origin
     * camera.SetOrthographic(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);
     * @endcode
     */
    void SetOrthographic(float left, float right, float bottom, float top, float nearPlane = -1.0f,
                         float farPlane = 1.0f);

    // ========== Getters ==========

    /**
     * @brief Get the camera's current world position
     * @return Current position vector
     */
    glm::vec3 GetPosition() const {
        return Position;
    }

    /**
     * @brief Get the camera's front/forward direction vector
     * @return Normalized front vector
     */
    glm::vec3 GetFront() const {
        return Front;
    }

    /**
     * @brief Get the camera's up direction vector
     * @return Normalized up vector
     */
    glm::vec3 GetUp() const {
        return Up;
    }

    /**
     * @brief Get the camera's right direction vector
     * @return Normalized right vector
     */
    glm::vec3 GetRight() const {
        return Right;
    }

    /**
     * @brief Get the camera's current yaw angle
     * @return Yaw angle in degrees
     */
    float GetYaw() const {
        return Yaw;
    }

    /**
     * @brief Get the camera's current pitch angle
     * @return Pitch angle in degrees
     */
    float GetPitch() const {
        return Pitch;
    }

    /**
     * @brief Get the camera's current roll angle
     * @return Roll angle in degrees
     */
    float GetRoll() const {
        return Roll;
    }

    /**
     * @brief Get the camera's current zoom level (FOV for perspective)
     * @return Zoom value (field of view in degrees for perspective cameras)
     */
    float GetZoom() const {
        return Zoom;
    }

    /**
     * @brief Get the camera's current aspect ratio
     * @return Aspect ratio (width/height)
     */
    float GetAspectRatio() const {
        return AspectRatio;
    }

    // ========== Utility Functions ==========

    /**
     * @brief Reset camera to default state
     *
     * Resets the camera's position, rotation, and settings to their default values.
     * Useful for "home" or "reset view" functionality.
     *
     * @note This resets:
     *       - Position to (0, 0, 0)
     *       - Rotation to default angles
     *       - Zoom to default FOV
     *       - Other parameters to default values
     */
    void Reset();

    /**
     * @brief Update the camera's aspect ratio
     *
     * Updates the aspect ratio used for perspective projection.
     * Should be called whenever the window/viewport size changes.
     *
     * @param aspectRatio New aspect ratio (width/height)
     *
     * @note Only affects perspective projection cameras
     *
     * @example
     * @code
     * // In window resize callback
     * void window_size_callback(GLFWwindow* window, int width, int height) {
     *     camera.UpdateAspectRatio(static_cast<float>(width) / height);
     *     glViewport(0, 0, width, height);
     * }
     * @endcode
     */
    void UpdateAspectRatio(float aspectRatio);

    // ========== Game-Specific Features ==========

    /**
     * @brief Convert screen coordinates to world space ray
     *
     * Converts 2D screen/mouse coordinates to a 3D ray in world space.
     * Useful for mouse picking, shooting mechanics, and object selection.
     * The ray starts at the camera position and extends through the world.
     *
     * @param screenX X coordinate on screen (pixels)
     * @param screenY Y coordinate on screen (pixels)
     * @param screenWidth Width of the screen/viewport (pixels)
     * @param screenHeight Height of the screen/viewport (pixels)
     * @return Normalized direction vector of the ray in world space
     *
     * @note The returned vector is the ray direction, not a point.
     *       To get points along the ray: point = camera.GetPosition() + t * ray
     *
     * @example
     * @code
     * // Cast ray from mouse cursor
     * double mouseX, mouseY;
     * glfwGetCursorPos(window, &mouseX, &mouseY);
     *
     * glm::vec3 ray = camera.GetScreenToWorldRay(
     *     static_cast<float>(mouseX), static_cast<float>(mouseY),
     *     windowWidth, windowHeight
     * );
     *
     * // Use ray for collision detection
     * if (RayIntersectsObject(camera.GetPosition(), ray, object)) {
     *     // Handle object selection
     * }
     * @endcode
     */
    glm::vec3 GetScreenToWorldRay(float screenX, float screenY, int screenWidth, int screenHeight) const;

    /**
     * @brief Check if a point is within the camera's view frustum
     *
     * Performs view frustum culling to determine if a point (optionally with radius)
     * is visible to the camera. Used for performance optimization by skipping
     * rendering of objects outside the camera's view.
     *
     * @param point World position to test
     * @param radius Optional radius around the point (for sphere culling)
     * @return true if the point/sphere is visible, false if outside view frustum
     *
     * @note This tests against all six planes of the view frustum:
     *       near, far, left, right, top, bottom
     *
     * @example
     * @code
     * // Cull objects outside camera view
     * for (const auto& object : gameObjects) {
     *     if (camera.IsInView(object.position, object.boundingRadius)) {
     *         object.Render();
     *     }
     * }
     * @endcode
     */
    bool IsInView(const glm::vec3 &point, float radius = 0.0f) const;

private:
    /**
     * @brief Update camera direction vectors from Euler angles
     *
     * Internal function that recalculates the Front, Right, and Up vectors
     * based on the current Yaw, Pitch, and Roll angles. Called automatically
     * whenever the camera's rotation is modified.
     *
     * @note This function maintains the orthogonal relationship between
     *       the three direction vectors and ensures they are normalized.
     */
    void UpdateCameraVectors();
};

} // namespace agl

#endif // CAMERA_H
