# AGL Game Engine - Sandbox Demo Project
cmake_minimum_required(VERSION 3.14)

include_directories(${INCLUDE_OUTPUT_PATH})
link_directories(${LIBRARY_OUTPUT_PATH})

# Determine which demo to build
if(NOT DEMO_NAME)
    set(DEMO_NAME "renderer" CACHE STRING "Name of demo to build")
endif()

# Set demo source file
set(DEMO_SOURCE "src/${DEMO_NAME}_demo.cpp")

# Check if demo source exists
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DEMO_SOURCE}")
    message(FATAL_ERROR "Demo source file '${DEMO_SOURCE}' not found!")
endif()

# Create executable for the specific demo
add_executable(agl_${DEMO_NAME}_demo
    ${DEMO_SOURCE}
)

# Also create a generic 'sandbox' target that points to the chosen demo
add_executable(sandbox
    ${DEMO_SOURCE}
)

# Create projectile demo executable
add_executable(agl_projectile_demo
    src/projectile_demo.cpp
)

# Set target properties for both executables
foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    # Include directories
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endforeach()

# Link gamelib
foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
    target_link_libraries(${target} PRIVATE gamelib)
endforeach()

# Copy assets and resources
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${target}>/assets
        )
    endforeach()
endif()

# Copy imgui.ini if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/imgui.ini")
    foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/imgui.ini
            $<TARGET_FILE_DIR:${target}>/imgui.ini
        )
    endforeach()
endif()

# Visual Studio specific settings
if(WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
    foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
        set_target_properties(${target} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        )
    endforeach()
endif()

# Check if gamelib target exists (when built as part of main project)
if(TARGET gamelib)
    # Link the gamelib library (this automatically includes all dependencies)
    foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
        target_link_libraries(${target} PRIVATE gamelib)
    endforeach()
    message(STATUS "Using gamelib target from parent project")
else()
    # Try to find pre-built gamelib first
    find_package(agl-gamelib QUIET)

    if(agl-gamelib_FOUND)
        # Use pre-built gamelib library
        foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
            target_link_libraries(${target} PRIVATE AGL::gamelib)
        endforeach()
        message(STATUS "Using pre-built gamelib from: ${agl-gamelib_DIR}")
    else()
        # Fallback: Standalone build by adding gamelib as subdirectory
        message(STATUS "Standalone build: adding gamelib as subdirectory")

        # Include parent project dependencies
        include(../cmake/Dependency.cmake)

        # Add gamelib
        add_subdirectory(../gamelib gamelib_build)

        # Link the gamelib library
        foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
            target_link_libraries(${target} PRIVATE gamelib)
        endforeach()
    endif()
endif()

# Additional include directories for demo-specific code
foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endforeach()

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
    foreach(target agl_${DEMO_NAME}_demo sandbox agl_projectile_demo)
        set_target_properties(${target} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
    endforeach()
endif()

# Display build information
message(STATUS "=== AGL Sandbox Build Configuration ===")
message(STATUS "Demo: ${DEMO_NAME}")
message(STATUS "Source: ${DEMO_SOURCE}")
message(STATUS "Output: agl_${DEMO_NAME}_demo")
message(STATUS "Target depends on: gamelib")
message(STATUS "=======================================")
