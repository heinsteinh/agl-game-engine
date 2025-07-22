# AGL Game Engine Library
cmake_minimum_required(VERSION 3.14)

# Collect all source files (exclude main.cpp since this is a library)
file(GLOB_RECURSE GAMELIB_SOURCES "src/*.cpp")
list(FILTER GAMELIB_SOURCES EXCLUDE REGEX "main\\.cpp$")

# Collect all header files
file(GLOB_RECURSE GAMELIB_HEADERS "include/*.h")

# Create the static library
add_library(gamelib STATIC ${GAMELIB_SOURCES} ${GAMELIB_HEADERS})

# Set target properties
set_target_properties(gamelib PROPERTIES
    OUTPUT_NAME "agl_gamelib"
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)

# Include directories (PUBLIC so that consumers can use them)
target_include_directories(gamelib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link external dependencies
target_link_libraries(gamelib PUBLIC
    glfw
    glm::glm
    stb
    imgui
    spdlog::spdlog
)

# Find and link OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(gamelib PUBLIC OpenGL::GL)

# Platform-specific configurations
if(APPLE)
    target_link_libraries(gamelib PUBLIC "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(gamelib PUBLIC X11 Xrandr Xinerama Xcursor GL dl pthread)
endif()

# Define compile definitions
target_compile_definitions(gamelib PUBLIC
    GL_GLEXT_PROTOTYPES
    AGL_ENGINE_LIBRARY=1
)

# Compiler-specific options
if(MSVC)
    target_compile_options(gamelib PRIVATE /W4)
else()
    target_compile_options(gamelib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(gamelib PUBLIC AGL_DEBUG=1)
endif()

message(STATUS "=== AGL GameLib Configuration ===")
message(STATUS "Library: gamelib (static)")
message(STATUS "Sources: ${GAMELIB_SOURCES}")
message(STATUS "Headers: ${GAMELIB_HEADERS}")
message(STATUS "==============================")