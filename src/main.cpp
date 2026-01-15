#include "Window.h"
#include "Viewer.h"
#include <iostream>
#include <filesystem>
#include <stdexcept>

void pause_and_exit(int exit_code)
{
    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.get();
    exit(exit_code);
}

int main(int argc, char *argv[])
{
    try
    {
        std::cout << "Initializing Window..." << std::endl;
        Window window(800, 600, "3D Viewer");
        if (!window.init())
        {
            std::cerr << "FATAL: Window initialization failed." << std::endl;
            pause_and_exit(-1);
        }

        std::cout << "Initializing Viewer..." << std::endl;
        Viewer viewer(800, 600);

        std::cout << "Linking event handler..." << std::endl;
        window.setEventHandler(&viewer);

        std::cout << "Initializing viewer subsystems..." << std::endl;
        viewer.init();

        std::cout << "Loading model..." << std::endl;
        std::string modelPath = "examples/skyscraper.obj";

        if (argc > 1)
        {
            modelPath = argv[1];
        }

        if (!std::filesystem::exists(modelPath))
        {
            std::cerr << "Warning: Could not find specified model path: " << std::filesystem::absolute(modelPath) << std::endl;
            std::cerr << "Usage: " << argv[0] << " <path_to_model>" << std::endl;
        }

        viewer.loadModel(modelPath);

        std::cout << "Entering main loop..." << std::endl;
        while (!window.shouldClose())
        {
            viewer.render();
            window.swapBuffers();
            window.pollEvents();
        }

        std::cout << "Exiting." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FATAL: An unhandled exception occurred: " << e.what() << std::endl;
        pause_and_exit(-2);
    }
    catch (...)
    {
        std::cerr << "FATAL: An unknown unhandled exception occurred." << std::endl;
        pause_and_exit(-3);
    }

    return 0;
}
