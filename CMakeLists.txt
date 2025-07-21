# Setup basic c++17 cross-platform for game engine project with FetchContent dependencies
cmake_minimum_required(VERSION 3.14)  # FetchContent requires 3.14+
project(TutoFromBook VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the output directories for the compiled binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add dependency management first
include(Dependency.cmake)

# Add the source files
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

# Only create executable if we have source files
if(SOURCE_FILES)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})

    # Include directories
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    # Link libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glm::glm
        stb
        imgui
    )

    # Find and link OpenGL
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

    # Platform-specific configurations
    if(APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE X11 Xrandr Xinerama Xcursor GL dl pthread)
    endif()

    # Define GL_GLEXT_PROTOTYPES to enable OpenGL function declarations
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_GLEXT_PROTOTYPES)
else()
    message(WARNING "No source files found in src/ directory")
endif()
