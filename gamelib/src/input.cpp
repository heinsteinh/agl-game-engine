#include "input.h"
#include <iostream>

namespace agl {

Input::Input()
    : m_window(nullptr)
    , m_mouseX(0.0)
    , m_mouseY(0.0)
    , m_previousMouseX(0.0)
    , m_previousMouseY(0.0)
    , m_mouseDeltaX(0.0)
    , m_mouseDeltaY(0.0)
    , m_scrollX(0.0)
    , m_scrollY(0.0) {
}

Input::~Input() {
    // Cleanup if needed
}

void Input::Initialize(GLFWwindow* window) {
    m_window = window;

    if (m_window) {
        // Set window user pointer for callbacks
        glfwSetWindowUserPointer(m_window, this);

        // Set GLFW callbacks
        glfwSetKeyCallback(m_window, GLFWKeyCallback);
        glfwSetMouseButtonCallback(m_window, GLFWMouseButtonCallback);
        glfwSetCursorPosCallback(m_window, GLFWCursorPositionCallback);
        glfwSetScrollCallback(m_window, GLFWScrollCallback);

        // Get initial mouse position
        glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
        m_previousMouseX = m_mouseX;
        m_previousMouseY = m_mouseY;
    }
}

void Input::Update() {
    // Update previous states
    m_previousKeyStates = m_keyStates;
    m_previousMouseButtonStates = m_mouseButtonStates;

    // Update mouse delta
    m_mouseDeltaX = m_mouseX - m_previousMouseX;
    m_mouseDeltaY = m_mouseY - m_previousMouseY;
    m_previousMouseX = m_mouseX;
    m_previousMouseY = m_mouseY;

    // Reset scroll offset (it's only valid for one frame)
    m_scrollX = 0.0;
    m_scrollY = 0.0;
}

bool Input::IsKeyPressed(int key) const {
    auto current = m_keyStates.find(key);
    auto previous = m_previousKeyStates.find(key);

    bool currentPressed = (current != m_keyStates.end() && current->second == KeyState::Pressed);
    bool previousPressed = (previous != m_previousKeyStates.end() && previous->second == KeyState::Pressed);

    return currentPressed && !previousPressed;
}

bool Input::IsKeyReleased(int key) const {
    auto current = m_keyStates.find(key);
    auto previous = m_previousKeyStates.find(key);

    bool currentPressed = (current != m_keyStates.end() && current->second == KeyState::Pressed);
    bool previousPressed = (previous != m_previousKeyStates.end() && previous->second == KeyState::Pressed);

    return !currentPressed && previousPressed;
}

bool Input::IsKeyHeld(int key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && (it->second == KeyState::Pressed || it->second == KeyState::Repeat);
}

bool Input::IsMouseButtonPressed(MouseButton button) const {
    int buttonCode = static_cast<int>(button);
    auto current = m_mouseButtonStates.find(buttonCode);
    auto previous = m_previousMouseButtonStates.find(buttonCode);

    bool currentPressed = (current != m_mouseButtonStates.end() && current->second == KeyState::Pressed);
    bool previousPressed = (previous != m_previousMouseButtonStates.end() && previous->second == KeyState::Pressed);

    return currentPressed && !previousPressed;
}

bool Input::IsMouseButtonReleased(MouseButton button) const {
    int buttonCode = static_cast<int>(button);
    auto current = m_mouseButtonStates.find(buttonCode);
    auto previous = m_previousMouseButtonStates.find(buttonCode);

    bool currentPressed = (current != m_mouseButtonStates.end() && current->second == KeyState::Pressed);
    bool previousPressed = (previous != m_previousMouseButtonStates.end() && previous->second == KeyState::Pressed);

    return !currentPressed && previousPressed;
}

bool Input::IsMouseButtonHeld(MouseButton button) const {
    int buttonCode = static_cast<int>(button);
    auto it = m_mouseButtonStates.find(buttonCode);
    return it != m_mouseButtonStates.end() && it->second == KeyState::Pressed;
}

void Input::GetMousePosition(double& x, double& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

double Input::GetMouseX() const {
    return m_mouseX;
}

double Input::GetMouseY() const {
    return m_mouseY;
}

void Input::GetMouseDelta(double& deltaX, double& deltaY) const {
    deltaX = m_mouseDeltaX;
    deltaY = m_mouseDeltaY;
}

void Input::GetScrollOffset(double& xOffset, double& yOffset) const {
    xOffset = m_scrollX;
    yOffset = m_scrollY;
}

void Input::SetCursorMode(int mode) {
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR, mode);
    }
}

// Static GLFW callbacks
void Input::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        KeyState state = static_cast<KeyState>(action);
        input->m_keyStates[key] = state;

        if (input->m_keyCallback) {
            input->m_keyCallback(key, state);
        }
    }
}

void Input::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        KeyState state = static_cast<KeyState>(action);
        input->m_mouseButtonStates[button] = state;

        if (input->m_mouseButtonCallback) {
            MouseButton mouseButton = static_cast<MouseButton>(button);
            input->m_mouseButtonCallback(mouseButton, state);
        }
    }
}

void Input::GLFWCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        input->m_mouseX = xpos;
        input->m_mouseY = ypos;

        if (input->m_mouseMoveCallback) {
            input->m_mouseMoveCallback(xpos, ypos);
        }
    }
}

void Input::GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        input->m_scrollX = xoffset;
        input->m_scrollY = yoffset;

        if (input->m_scrollCallback) {
            input->m_scrollCallback(xoffset, yoffset);
        }
    }
}

}
