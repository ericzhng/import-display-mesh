#include "Window.h"
#include "Viewer.h"
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include "Style.h"

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
        Window window(1000, 800, "3D Viewer");
        if (!window.init())
        {
            std::cerr << "FATAL: Window initialization failed." << std::endl;
            pause_and_exit(-1);
        }

        window.initImGui(); // Initialize ImGui

        Viewer viewer(1000, 800);
        window.setEventHandler(&viewer);
        viewer.init();

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

        // No more std::cout after this point for cleaner output
        // std::cout << "Entering main loop..." << std::endl;
        while (!window.shouldClose())
        {
            if (updateThemeIfChanged())
            {
                viewer.setBackgroundTheme(g_isDarkMode);
            }
            window.pollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            viewer.render(); // Render scene and potentially ImGui elements

            // ImGui rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            ImGuiIO &io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow *backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            window.swapBuffers();
        }

        window.shutdownImGui(); // Shutdown ImGui

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
