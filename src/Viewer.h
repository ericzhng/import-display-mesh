#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Background.h"
#include "AxesWidget.h"
#include "IEventHandler.h"
#include "UiRenderer.h" // Include UiRenderer

class Viewer : public IEventHandler
{
public:
    Viewer(int width, int height);
    ~Viewer();

    void init();
    void loadModel(const std::string &path);
    void render();

    // Input Handling - implementing IEventHandler
    void onResize(int width, int height) override;
    void onKey(int key, int action) override;
    void onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton) override;
    void onScroll(float yoffset) override;
    void onMouseButton(int button, int action, double xpos, double ypos) override;
    void drawContextMenu();
    void setPerspective(bool enable) { usePerspective = enable; }

private:
    int width, height;
    Camera camera;

    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Model> model;
    std::unique_ptr<Background> background;
    std::unique_ptr<AxesWidget> axesWidget;
    std::unique_ptr<UiRenderer> uiRenderer; // Add UiRenderer member

    // State
    bool usePerspective;
    float lastX, lastY;
    bool firstMouse;
    bool m_show_edges = false; // Added for wireframe toggling
    bool m_lightingEnabled;    // New: To toggle lighting
    float m_ambientStrength;   // New: To control ambient light strength

    // Context menu state
    bool m_showContextMenu = false;
    float m_contextMenuX = 0.0f;
    float m_contextMenuY = 0.0f;

    // Helper for text rendering (stub)
    void drawTextStub(float x, float y, const std::string& text, float fontSize, glm::vec4 color);
};
