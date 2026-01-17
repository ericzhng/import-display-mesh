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
#include "UiRenderer.h"   // Include UiRenderer
#include "TextRenderer.h" // Include TextRenderer

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
    void setPerspective(bool enable) { usePerspective = enable; }
    void setBackgroundTheme(bool isDarkTheme);

private:
    int width, height;
    Camera camera;
    bool m_isDarkTheme = false;

    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Model> model;
    std::unique_ptr<Background> background;
    std::unique_ptr<AxesWidget> axesWidget;
    std::unique_ptr<UiRenderer> uiRenderer;     // Add UiRenderer member
    std::unique_ptr<TextRenderer> textRenderer; // Add TextRenderer member

    // Enum for model view modes
    enum class ModelViewMode
    {
        Shaded,
        Wireframe,
        ShadedWithEdges
    };

    // State
    bool usePerspective;
    float lastX, lastY;
    bool firstMouse;
    ModelViewMode m_modelViewMode = ModelViewMode::Shaded; // Replaced m_show_edges
    bool m_lightingEnabled;                                // New: To toggle lighting
    float m_ambientStrength;                               // New: To control ambient light strength
    bool m_showAxesWidget = true;                          // New: To toggle axes widget visibility
    bool m_showDebugWindow = false;                        // New: To toggle debug window visibility
    bool m_showCameraWindow = false;                       // New: To toggle camera window visibility
    bool m_cameraControlWindowFirstOpen = true;            // New: To track if Camera Control window is opened for the first time
    bool m_cameraDebugWindowFirstOpen = true;              // New: To track if Camera Debug window is opened for the first time

    // Camera Animation State
    bool m_isAnimatingCamera = false;
    float m_animationDuration = 0.5f; // Animation duration in seconds
    float m_animationTime = 0.0f;

    glm::vec3 m_cameraAnimStartPosition;
    glm::vec3 m_cameraAnimStartTarget;
    glm::vec3 m_cameraAnimStartWorldUp;

    glm::vec3 m_cameraAnimEndPosition;
    glm::vec3 m_cameraAnimEndTarget;
    glm::vec3 m_cameraAnimEndWorldUp;

    // Time tracking for animation
    float m_lastFrameTime = 0.0f;
    float m_deltaTime = 0.0f;

    // Model Drop Animation State
    bool m_isAnimatingModelDrop = false;
    float m_modelDropDuration = 1.0f; // Duration of the model drop animation
    float m_modelDropTime = 0.0f;
    glm::vec3 m_modelAnimStartPosition; // Starting position (elevated)
    glm::vec3 m_modelAnimEndPosition;   // Ending position (actual center)
    glm::vec3 m_currentModelPosition;   // The model's current animated position

    // Camera control variables
    float m_focalLength = 50.0f;
    float m_sensorHeight = 24.0f; // Assuming 35mm full-frame equivalent sensor height

    float m_customAspectRatio = 16.0f / 9.0f;


    void autoCenterAndOrientModel();
    // ... existing members ...
    float getUiScale() const;
};
