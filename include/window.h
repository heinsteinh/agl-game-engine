#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace agl {
    class Window {
    public:
        using ResizeCallback = std::function<void(int, int)>;
        using CloseCallback = std::function<void()>;

        Window();
        ~Window();

        bool Create(int width, int height, const char* title);
        void Destroy();

        void SwapBuffers();
        void PollEvents();
        bool ShouldClose() const;
        void SetShouldClose(bool shouldClose);

        // Getters
        GLFWwindow* GetNativeWindow() const { return m_window; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        const std::string& GetTitle() const { return m_title; }

        // Setters
        void SetTitle(const char* title);
        void SetSize(int width, int height);
        void SetVSync(bool enabled);

        // Callbacks
        void SetResizeCallback(const ResizeCallback& callback) { m_resizeCallback = callback; }
        void SetCloseCallback(const CloseCallback& callback) { m_closeCallback = callback; }

        // Static methods
        static void Initialize();
        static void Terminate();

    private:
        GLFWwindow* m_window;
        int m_width;
        int m_height;
        std::string m_title;
        bool m_vsyncEnabled;

        ResizeCallback m_resizeCallback;
        CloseCallback m_closeCallback;

        // GLFW callbacks
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
        static void WindowCloseCallback(GLFWwindow* window);
        static void ErrorCallback(int error, const char* description);
    };
}

#endif // WINDOW_H
