#include "Window.h"
#include "Viewer.h"
#include <iostream>

int main()
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

    // 5. Load default model
    // Try to locate the file, assuming running from root or build/
    std::string modelPath = "E:/2026-01/OpenGL/import-display-mesh/examples/airboat.obj";
    FILE *f = fopen(modelPath.c_str(), "r");
    if (!f)
    {
        // Try going up one level (e.g. running from build/)
        modelPath = "../examples/airboat.obj";
        f = fopen(modelPath.c_str(), "r");
        if (!f)
        {
            std::cerr << "Warning: Could not find default model at examples/airboat.obj" << std::endl;
        }
        else
        {
            fclose(f);
            viewer.loadModel(modelPath);
        }
    }
    else
    {
        fclose(f);
        viewer.loadModel(modelPath);
    }

    // 6. Main Loop
    while (!window.shouldClose())
    {
        viewer.render();
        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
