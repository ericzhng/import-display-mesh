# Project Objective

The goal of this project is to develop a lightweight, high-performance C++ 3D Model Viewer using OpenGL. This will be driven by data from simulation. The target is that this Model Viewer can visualize the simulation results passed from either CPU or CUDA memory.

## Key Features
- **Model Loading:** Utilizes the Assimp library to import various 3D model formats (e.g., .obj) with support for vertices, normals, and indices.
- **Rendering Engine:** Implements a modern OpenGL rendering pipeline (VBOs, VAOs) with custom shaders for model and background rendering.
- **Interactive Viewing:** Features a flexible camera system allowing users to inspect models via mouse interactions (rotate, pan, zoom).
- **Visualization Aids:** Includes coordinate axes and a gradient background to enhance spatial understanding of structural meshes.
- **Cross-Platform:** Built on top of GLFW and GLAD for cross-platform windowing and OpenGL context management.

## Technical Stack
- **Language:** C++
- **Graphics API:** OpenGL
- **Libraries:**
    - `GLFW`: Window creation and input handling.
    - `GLAD`: OpenGL extension loading.
    - `GLM`: Mathematics (vectors, matrices).
    - `Assimp`: 3D model import.

## Directory Structure
*   `src/`: Source code files (`main.cpp`, `Model.h`, `Mesh.h`, `Camera.h`, `Camera.cpp`, `Window.h`, `Window.cpp`, `Shader.h`, `View.h`, `View.cpp`).
*   `shaders/`: GLSL shader source files (`shader.vs`, `shader.fs`, `background.vs`, `background.fs`).
*   `vendor/`: Third-party libraries (contains `glm`).
*   `examples/`: Sample 3D models (e.g., `cube.obj`).
*   `CMakeLists.txt`: CMake build configuration.

## Building and Running

### Prerequisites
*   CMake (3.10 or higher)
*   C++ Compiler (supporting C++17, In Windows, using Visual Studio Build Tools)
*   VCPKG (latest version, if in Windows)
*   Development libraries for GLFW, and Assimp must be installed and findable by CMake.

### Build Instructions
1.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```
2.  Generate build files:
    ```bash
    cmake -CMAKE_TOOLCHAIN_FILE=$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake ..
    ```
3.  Compile the project:
    ```bash
    cmake --build .
    ```
Do not run cmake or tests to verify the build. I will run it manually for you.

### Running the Application
The executable `MeshViewer` will be located in the `build` directory (or `build/Debug` on Windows).

**Important:** The application expects to find the `shaders/` directory relative to the working directory. CMake will automatically copy the `shaders` folder to the executable's directory.

## Controls
*   **Left Mouse Drag:** Orbit Camera
*   **Middle Mouse Button Hold:** Pan Camera
*   **Middle Mouse Scroll:** Zoom In/Out

## Codebase Conventions
*   **Model Loading:** The `Model` class (`src/Model.h`) uses Assimp to process nodes and meshes recursively.
*   **Mesh Rendering:** The `Mesh` class (`src/Mesh.h`) handles OpenGL buffers (VAO, VBO, EBO) and drawing calls.
*   **Shaders:** `Shader.h` is used to load, compile, and link vertex and fragment shaders.
*   **Coordinate System:** Uses a right-handed coordinate system with a perspective projection.
