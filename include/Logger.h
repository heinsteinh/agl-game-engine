#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

namespace agl {
    class Logger {
    public:
        static void Initialize();
        static void Shutdown();

        // Get the core logger
        static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        
        // Get the client logger
        static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}

// Core log macros (for engine internal logging)
#define AGL_CORE_TRACE(...)    ::agl::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define AGL_CORE_INFO(...)     ::agl::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define AGL_CORE_WARN(...)     ::agl::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define AGL_CORE_ERROR(...)    ::agl::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define AGL_CORE_CRITICAL(...) ::agl::Logger::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros (for application/game logging)
#define AGL_TRACE(...)         ::agl::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define AGL_INFO(...)          ::agl::Logger::GetClientLogger()->info(__VA_ARGS__)
#define AGL_WARN(...)          ::agl::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define AGL_ERROR(...)         ::agl::Logger::GetClientLogger()->error(__VA_ARGS__)
#define AGL_CRITICAL(...)      ::agl::Logger::GetClientLogger()->critical(__VA_ARGS__)

#endif // LOGGER_H
