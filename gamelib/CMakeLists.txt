# AGL Game Engine Library
cmake_minimum_required(VERSION 3.16)

# Collect all source files (exclude main.cpp since this is a library)
file(GLOB_RECURSE GAMELIB_SOURCES "src/*.cpp")
list(FILTER GAMELIB_SOURCES EXCLUDE REGEX "main\\.cpp$")

# Collect all header files
file(GLOB_RECURSE GAMELIB_HEADERS
    "include/*.h"
    "include/*.hpp"
    "src/*.h"
    "src/*.hpp"
)

include_directories(${INCLUDE_OUTPUT_PATH})



# Create the static library
add_library(gamelib STATIC 
 ${GAMELIB_SOURCES}
 ${GAMELIB_HEADERS})


 # Set target properties
set_target_properties(gamelib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Set target properties
set_target_properties(gamelib PROPERTIES
    OUTPUT_NAME "agl_gamelib"
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)

# Include directories (PUBLIC so that consumers can use them)
#target_include_directories(gamelib PUBLIC
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_SOURCE_DIR}/src
#)

# Public include directories
target_include_directories(gamelib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Private include directories
target_include_directories(gamelib PRIVATE
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

# Preprocessor definitions
target_compile_definitions(gamelib PUBLIC
    $<$<CONFIG:Debug>:AGL_DEBUG>
    $<$<CONFIG:Release>:AGL_RELEASE>
    AGL_ENGINE_VERSION_MAJOR=1
    AGL_ENGINE_VERSION_MINOR=0
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(gamelib PUBLIC AGL_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(gamelib PUBLIC AGL_PLATFORM_MACOS)
else()
    target_compile_definitions(gamelib PUBLIC AGL_PLATFORM_LINUX)
endif()

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

# Export targets for find_package
install(TARGETS gamelib EXPORT gamelib-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "=== AGL GameLib Configuration ===")
message(STATUS "Library: gamelib (static)")
message(STATUS "Sources: ${GAMELIB_SOURCES}")
message(STATUS "Headers: ${GAMELIB_HEADERS}")
message(STATUS "==============================")
