#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>

namespace agl {
    enum class KeyState {
        Released = GLFW_RELEASE,
        Pressed = GLFW_PRESS,
        Repeat = GLFW_REPEAT
    };

    enum class MouseButton {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE
    };

    class Input {
    public:
        using KeyCallback = std::function<void(int, KeyState)>;
        using MouseButtonCallback = std::function<void(MouseButton, KeyState)>;
        using MouseMoveCallback = std::function<void(double, double)>;
        using ScrollCallback = std::function<void(double, double)>;

        Input();
        ~Input();

        void Initialize(GLFWwindow* window);
        void Update();

        // Key input
        bool IsKeyPressed(int key) const;
        bool IsKeyReleased(int key) const;
        bool IsKeyHeld(int key) const;

        // Mouse input
        bool IsMouseButtonPressed(MouseButton button) const;
        bool IsMouseButtonReleased(MouseButton button) const;
        bool IsMouseButtonHeld(MouseButton button) const;

        // Mouse position
        void GetMousePosition(double& x, double& y) const;
        double GetMouseX() const;
        double GetMouseY() const;

        // Mouse delta (movement since last frame)
        void GetMouseDelta(double& deltaX, double& deltaY) const;

        // Scroll
        void GetScrollOffset(double& xOffset, double& yOffset) const;

        // Callbacks
        void SetKeyCallback(const KeyCallback& callback) { m_keyCallback = callback; }
        void SetMouseButtonCallback(const MouseButtonCallback& callback) { m_mouseButtonCallback = callback; }
        void SetMouseMoveCallback(const MouseMoveCallback& callback) { m_mouseMoveCallback = callback; }
        void SetScrollCallback(const ScrollCallback& callback) { m_scrollCallback = callback; }

        // Cursor modes
        void SetCursorMode(int mode); // GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED

    private:
        GLFWwindow* m_window;

        // Key states
        std::unordered_map<int, KeyState> m_keyStates;
        std::unordered_map<int, KeyState> m_previousKeyStates;

        // Mouse states
        std::unordered_map<int, KeyState> m_mouseButtonStates;
        std::unordered_map<int, KeyState> m_previousMouseButtonStates;

        // Mouse position
        double m_mouseX, m_mouseY;
        double m_previousMouseX, m_previousMouseY;
        double m_mouseDeltaX, m_mouseDeltaY;

        // Scroll
        double m_scrollX, m_scrollY;

        // Callbacks
        KeyCallback m_keyCallback;
        MouseButtonCallback m_mouseButtonCallback;
        MouseMoveCallback m_mouseMoveCallback;
        ScrollCallback m_scrollCallback;

        // GLFW callbacks (static methods with different names to avoid conflicts)
        static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void GLFWCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
        static void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    };
}
