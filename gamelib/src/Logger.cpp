#include "Logger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

namespace agl {
std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

void Logger::Initialize() {
    std::vector<spdlog::sink_ptr> logSinks;

    // Console sink with color
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    // File sink
    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("agl-engine.log", true));

    // Set pattern for both sinks
    logSinks[0]->set_pattern("%^[%T] %n: %v%$");  // Console with color
    logSinks[1]->set_pattern("[%T] [%l] %n: %v"); // File without color

    // Core logger (for engine internal logging)
    s_CoreLogger = std::make_shared<spdlog::logger>("AGL", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_CoreLogger);
    s_CoreLogger->set_level(spdlog::level::trace);
    s_CoreLogger->flush_on(spdlog::level::trace);

    // Client logger (for application/game logging)
    s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_ClientLogger);
    s_ClientLogger->set_level(spdlog::level::trace);
    s_ClientLogger->flush_on(spdlog::level::trace);
}

void Logger::Shutdown() {
    s_CoreLogger.reset();
    s_ClientLogger.reset();
    spdlog::shutdown();
}
} // namespace agl
