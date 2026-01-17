
cmake_minimum_required(VERSION 3.10)

project(MeshViewer LANGUAGES CXX)

add_definitions(
    -DIMGUI_ENABLE_DOCKING=1
    -DIMGUI_ENABLE_VIEWPORTS=1
)

# --- Set Standards ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Packages
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)

# Add GLM from the vendor directory. This creates the 'glm' target.
add_subdirectory(vendor/glm)

# --- Project Sources ---
# Gather all source and header files from the src directory.
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# # Example for GLM
# include(FetchContent)
# FetchContent_Declare(
#   glm
#   GIT_REPOSITORY https://github.com/g-truc/glm.git
#   GIT_TAG 1.0.3 # Or latest
# )
# FetchContent_MakeAvailable(glm)

# --- Executable Target ---
set_source_files_properties(vendor/glad/src/glad.c PROPERTIES LANGUAGE C)
set_source_files_properties(vendor/tinyfiledialogs/tinyfiledialogs.c PROPERTIES LANGUAGE C)
add_executable(MeshViewer
    ${SOURCES}
    src/UiRenderer.cpp
    src/TextRenderer.cpp # Explicitly add TextRenderer.cpp
    vendor/glad/src/glad.c
    vendor/tinyfiledialogs/tinyfiledialogs.c
    # ImGui Sources
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(MeshViewer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyfiledialogs"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends"
    ${ASSIMP_INCLUDE_DIRS}
)

target_link_libraries(MeshViewer PRIVATE
    glfw
    glm::glm
    assimp::assimp
    Freetype::Freetype
    Comdlg32
    Shell32
    User32
)

add_custom_command(TARGET MeshViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:MeshViewer>/shaders"
    COMMENT "Copying shaders to output directory..."
)

add_custom_command(TARGET MeshViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/examples"
    "$<TARGET_FILE_DIR:MeshViewer>/examples"
    COMMENT "Copying example models to output directory..."
)
