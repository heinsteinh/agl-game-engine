# Dependency.cmake - Fetch external dependencies using FetchContent

include(FetchContent)

# Set FetchContent properties
set(FETCHCONTENT_QUIET FALSE)

# GLFW - OpenGL window and input library
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
    GIT_PROGRESS TRUE
)

# GLM - OpenGL Mathematics library
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
    GIT_PROGRESS TRUE
)

# STB - Single-file public domain libraries
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_PROGRESS TRUE
)

# Dear ImGui - Immediate mode GUI library
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
    GIT_PROGRESS TRUE
)

# Make dependencies available
FetchContent_MakeAvailable(glfw glm stb imgui)

# Configure GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Create STB library target (header-only library)
if(stb_POPULATED AND NOT TARGET stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif()

# Create ImGui library target
if(imgui_POPULATED AND NOT TARGET imgui)
    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    add_library(imgui STATIC ${IMGUI_SOURCES})

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(imgui PUBLIC glfw)

    # Find OpenGL
    find_package(OpenGL REQUIRED)
    target_link_libraries(imgui PUBLIC OpenGL::GL)

    # Platform-specific configurations
    if(WIN32)
        target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
    elseif(APPLE)
        target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)
    else()
        target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
    endif()
endif()

# GLM is header-only, no additional setup needed
if(glm_POPULATED AND NOT TARGET glm::glm)
    # GLM creates its own targets, so we just ensure it's available
endif()
