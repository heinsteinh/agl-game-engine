#include "window.h"
#include <iostream>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace agl {

Window::Window()
    : m_window(nullptr), m_width(0), m_height(0), m_title(""), m_vsyncEnabled(true) {
}

Window::~Window() {
    Destroy();
}

void Window::Initialize() {
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void Window::Terminate() {
    glfwTerminate();
}

    bool Window::Create(int width, int height, const char *title) {
    m_width = width;
    m_height = height;
    m_title = title;

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(m_window);

    // Set window user pointer to this instance for callbacks
    glfwSetWindowUserPointer(m_window, this);

    // Set callbacks
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetWindowCloseCallback(m_window, WindowCloseCallback);

    // Enable VSync by default
    SetVSync(m_vsyncEnabled);

    // Set initial viewport
    glViewport(0, 0, width, height);

    return true;
}

    void Window::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Window::SwapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void Window::PollEvents() {
    glfwPollEvents();
}

bool Window::ShouldClose() const {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void Window::SetShouldClose(bool shouldClose) {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, shouldClose ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::SetTitle(const char *title) {
    if (m_window && title) {
        m_title = title;
        glfwSetWindowTitle(m_window, title);
    }
}

void Window::SetSize(int width, int height) {
    if (m_window) {
        m_width = width;
        m_height = height;
        glfwSetWindowSize(m_window, width, height);
    }
}

void Window::SetVSync(bool enabled) {
    m_vsyncEnabled = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

    // Static GLFW callbacks
void Window::FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    Window *windowInstance = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (windowInstance) {
        windowInstance->m_width = width;
        windowInstance->m_height = height;

        // Trigger signal events
        windowInstance->OnWindowResize.broadcast(width, height);

        // Legacy callback support
        if (windowInstance->m_resizeCallback) {
            windowInstance->m_resizeCallback(width, height);
        }
    }
}

void Window::WindowCloseCallback(GLFWwindow *window) {
    Window *windowInstance = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (windowInstance) {
        // Trigger signal events
        windowInstance->OnWindowClose.broadcast();

        // Legacy callback support
        if (windowInstance->m_closeCallback) {
            windowInstance->m_closeCallback();
        }
    }
}

void Window::ErrorCallback(int error, const char *description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

}
