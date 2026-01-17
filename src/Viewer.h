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
#include "CameraAnimator.h" // NEW: Include CameraAnimator header

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

class Viewer : public IEventHandler, public IAxesWidgetListener
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

    // IAxesWidgetListener implementation
    void OnAxesWidgetClick(Axis axis) override;

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
    bool m_showGridControlWindow = false;
    bool m_showSummaryWindow = false;                      // New: To toggle summary window visibility
    bool m_cameraControlWindowFirstOpen = true; // New: To track if Camera Control window is opened for the first time
    bool m_cameraDebugWindowFirstOpen = true;   // New: To track if Camera Debug window is opened for the first time

    // Time tracking for animation
    float m_lastFrameTime = 0.0f;
    float m_deltaTime = 0.0f;

    // Model Drop Animation State

    glm::vec3 m_modelAnimEndPosition; // Ending position (actual center)
    glm::vec3 m_currentModelPosition; // The model's current animated position

    // Grid properties
    std::unique_ptr<Shader> m_gridShader; // Shader for the grid
    std::unique_ptr<CameraAnimator> m_cameraAnimator; // NEW: Camera Animator
    bool m_showGrid = true;
    float m_gridSpacing = 10.0f;
    int m_gridExtent = 10; // Number of major grid lines from the center (e.g., 10 means from -10 to +10)
    glm::vec4 m_gridColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    unsigned int m_xAxisVAO = 0, m_xAxisVBO = 0;
    unsigned int m_yAxisVAO = 0, m_yAxisVBO = 0;
    unsigned int m_majorGridVAO = 0, m_majorGridVBO = 0;

    // Camera control variables
    float m_focalLength = 50.0f;
    float m_sensorHeight = 24.0f; // Assuming 35mm full-frame equivalent sensor height

    float m_customAspectRatio = 16.0f / 9.0f;
    bool m_useCustomAspectRatio = false;          // Re-added: Flag to use custom aspect ratio
    float m_lastWindowAspectRatio = 16.0f / 9.0f; // New: Stores the last calculated window aspect ratio
    float m_summaryWindowHeight = 50.0f;          // New: Height reserved for the summary window

    void autoCenterAndOrientModel();
    void drawGrid(int viewportWidth, int viewportHeight);
    void setupGrid();
    Axis m_lastAlignedAxis; // New: Stores the last intended aligned axis
    // ... existing members ...
    float getUiScale() const;
};
