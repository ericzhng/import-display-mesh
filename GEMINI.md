# MeshViewer

## Project Overview
**MeshViewer** is a C++ OpenGL application designed to import and display 3D structural meshes. It utilizes **Assimp** for model loading, **GLFW** for window management, **GLEW** for OpenGL extension loading, and **GLM** for mathematics.

The application currently loads a sample 3D model (hardcoded path) and renders it with a basic shader, allowing for camera navigation via mouse input.

## Key Technologies
*   **Language:** C++17
*   **Build System:** CMake
*   **Graphics API:** OpenGL
*   **Dependencies:**
    *   **GLFW3:** Windowing and input.
    *   **GLEW:** OpenGL function loading.
    *   **Assimp:** 3D model import (supports formats like OBJ, FBX, etc.).
    *   **GLM:** Mathematics (vectors, matrices).

## Directory Structure
*   `src/`: Source code files (`main.cpp`, `Model.h`, `Mesh.h`, `Camera.h`, `Shader.h`).
*   `shaders/`: GLSL shader source files (`shader.vs`, `shader.fs`).
*   `vendor/`: Third-party libraries (contains `glm`).
*   `examples/`: Sample 3D models (e.g., `cube.obj`).
*   `extra/`: Additional test files (`test_gl_windows.cpp`).
*   `CMakeLists.txt`: CMake build configuration.

## Building and Running

### Prerequisites
*   CMake (3.10 or higher)
*   C++ Compiler (supporting C++17)
*   Development libraries for GLFW, GLEW, and Assimp must be installed and findable by CMake.

### Build Instructions
1.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```
2.  Generate build files:
    ```bash
    cmake ..
    ```
3.  Compile the project:
    ```bash
    cmake --build .
    ```

### Running the Application
The executable `MeshViewer` will be located in the `build` directory (or `build/Debug` on Windows).

**Important:** The application expects to find the `shaders/` directory relative to the working directory. It is recommended to run the executable from the project root or copy the `shaders` folder to the executable's directory.

**Note:** The path to the 3D model is currently **hardcoded** in `src/main.cpp` line 69. You may need to modify this path to point to a valid file on your system (e.g., `examples/cube.obj` with an absolute path) before recompiling.

## Controls
*   **Middle Mouse Button + Shift:** Orbit Camera
*   **Middle Mouse Button:** Pan Camera
*   **Middle Mouse Scroll:** Zoom In/Out

## Codebase Conventions
*   **Model Loading:** The `Model` class (`src/Model.h`) uses Assimp to process nodes and meshes recursively.
*   **Mesh Rendering:** The `Mesh` class (`src/Mesh.h`) handles OpenGL buffers (VAO, VBO, EBO) and drawing calls.
*   **Shaders:** `Shader.h` is used to load, compile, and link vertex and fragment shaders.
*   **Coordinate System:** Uses a right-handed coordinate system with a perspective projection.
