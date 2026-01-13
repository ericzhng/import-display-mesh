
cmake_minimum_required(VERSION 3.10)

project(MeshViewer LANGUAGES CXX)

# --- Set Standards ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Packages
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

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
add_executable(MeshViewer ${SOURCES} ${HEADERS})

target_include_directories(MeshViewer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${GLEW_INCLUDE_DIRS} 
    ${ASSIMP_INCLUDE_DIRS}
)

target_link_libraries(MeshViewer PRIVATE
    GLEW::GLEW 
    glfw 
    glm::glm 
    assimp::assimp
)

add_custom_command(TARGET MeshViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:MeshViewer>/shaders"
    COMMENT "Copying shaders to output directory..."
)

# --- Executable Target ---
add_executable(extraTest extra/test_gl_windows.cpp)

target_include_directories(extraTest PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/extra"
    ${GLEW_INCLUDE_DIRS}
)

target_link_libraries(extraTest PRIVATE
    GLEW::GLEW 
    glfw 
    glm::glm 
)
