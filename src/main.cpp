#include "Window.h"
#include "Viewer.h"
#include <iostream>
#include <filesystem>

int main(int argc, char *argv[])
{
    // 1. Create Window (Handles GLFW init and Context creation)
    Window window(800, 600, "Structural Mesh Viewer");
    if (!window.init())
    {
        return -1;
    }

    // 2. Create Viewer
    Viewer viewer(800, 600);

    // 3. Link Viewer to Window for callbacks
    window.setUserPointer(&viewer);

    // 4. Initialize Viewer (Context is now valid)
    viewer.init();

    // 5. Load model
    // Default model path
    std::string modelPath = "E:/2026-01/OpenGL/import-display-mesh/examples/airboat.obj";

    // Check for command line argument
    if (argc > 1)
    {
        modelPath = argv[1];
    }

    if (!std::filesystem::exists(modelPath))
    {
        std::cerr << "Warning: Could not find specified model " << modelPath << std::endl;
        std::cerr << "Usage: " << argv[0] << " <path_to_model>" << std::endl;
    }

    viewer.loadModel(modelPath);

    // 6. Main Loop
    while (!window.shouldClose())
    {
        viewer.render();
        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
