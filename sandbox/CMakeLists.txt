# AGL Game Engine - Sandbox Demo Project
cmake_minimum_required(VERSION 3.14)

include_directories(${INCLUDE_OUTPUT_PATH})
link_directories(${LIBRARY_OUTPUT_PATH})

# Collect sandbox source files
file(GLOB_RECURSE SANDBOX_SOURCES
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE SANDBOX_HEADERS
    "src/*.h"
    "src/*.hpp"
)

# Create sandbox executable
add_executable(sandbox
    ${SANDBOX_SOURCES}
    ${SANDBOX_HEADERS}
)

# Set target properties
set_target_properties(sandbox PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)


# Include directories
target_include_directories(sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link gamelib
target_link_libraries(sandbox PRIVATE
    gamelib
)

# Copy assets and resources
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:sandbox>/assets
    )
endif()

# Copy imgui.ini if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/imgui.ini")
    add_custom_command(TARGET sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/imgui.ini
        $<TARGET_FILE_DIR:sandbox>/imgui.ini
    )
endif()

# Visual Studio specific settings
if(WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
    set_target_properties(sandbox PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    )
endif()

# Check if gamelib target exists (when built as part of main project)
if(TARGET gamelib)
    # Link the gamelib library (this automatically includes all dependencies)
    target_link_libraries(sandbox PRIVATE gamelib)
    message(STATUS "Using gamelib target from parent project")
else()
    # Try to find pre-built gamelib first
    find_package(agl-gamelib QUIET)

    if(agl-gamelib_FOUND)
        # Use pre-built gamelib library
        target_link_libraries(sandbox PRIVATE AGL::gamelib)
        message(STATUS "Using pre-built gamelib from: ${agl-gamelib_DIR}")
    else()
        # Fallback: Standalone build by adding gamelib as subdirectory
        message(STATUS "Standalone build: adding gamelib as subdirectory")

        # Include parent project dependencies
        include(../cmake/Dependency.cmake)

        # Add gamelib
        add_subdirectory(../gamelib gamelib_build)

        # Link the gamelib library
        target_link_libraries(sandbox PRIVATE gamelib)
    endif()
endif()

# Additional include directories for demo-specific code
target_include_directories(sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Copy assets to build directory if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(GLOB_RECURSE ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
    if(ASSETS)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        message(STATUS "Copied assets to build directory")
    endif()
endif()

# Set debug working directory for Visual Studio
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()

# Display build information
message(STATUS "=== AGL Sandbox Build Configuration ===")
message(STATUS "Demo: ${DEMO_NAME}")
message(STATUS "Source: ${DEMO_SOURCE}")
message(STATUS "Output: agl_${DEMO_NAME}_demo")
message(STATUS "Target depends on: gamelib")
message(STATUS "=======================================")
