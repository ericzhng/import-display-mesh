#include "Viewer.h"
#include "Background.h"
#include "AxesWidget.h"
#include "TextRenderer.h" // Include TextRenderer
#include "Style.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <tinyfiledialogs.h>

const float CAMERA_ANIMATION_DURATION = 0.5f; // Duration for camera animations

// Anonymous namespace for helper functions local to this translation unit
namespace
{
    // Helper to get the absolute cardinal axis (e.g., X_POS, Y_POS, Z_POS for respective axes)
    Axis getCardinalAxisEnum(Axis a)
    {
        switch (a)
        {
        case Axis::X_POS:
        case Axis::X_NEG:
            return Axis::X_POS; // Represent X-axis by X_POS
        case Axis::Y_POS:
        case Axis::Y_NEG:
            return Axis::Y_POS; // Represent Y-axis by Y_POS
        case Axis::Z_POS:
        case Axis::Z_NEG:
            return Axis::Z_POS; // Represent Z-axis by Z_POS
        default:
            return Axis::NONE;
        }
    }
} // end anonymous namespace

Viewer::Viewer(int width, int height)
    : width(width), height(height),
      camera(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      usePerspective(false), firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f),
      m_lightingEnabled(true), m_ambientStrength(0.1f), // Initialize new members
      m_lastFrameTime(0.0f),                            // Initialize m_lastFrameTime here
      m_currentModelPosition(0.0f, 0.0f, 0.0f),
      m_isDarkTheme(false), // Initialize m_isDarkTheme
      m_customAspectRatio(static_cast<float>(width) / height),
      m_lastAlignedAxis(Axis::NONE), // Initialize new member m_lastAlignedAxis
      m_cameraAnimator(std::make_unique<CameraAnimator>()) // NEW: Initialize CameraAnimator
{
    background = std::make_unique<Background>();
    uiRenderer = std::make_unique<UiRenderer>();                                      // Initialize UiRenderer
    axesWidget = std::make_unique<AxesWidget>(width, height, uiRenderer.get(), this); // Initialize AxesWidget with screen dimensions, uiRenderer, and listener
    textRenderer = std::make_unique<TextRenderer>(width, height);                     // Initialize TextRenderer
}

Viewer::~Viewer()
{
}

void Viewer::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_FRAMEBUFFER_SRGB); // Enable automatic sRGB color space conversion

    // Use absolute paths for shaders to ensure correct loading
    mainShader = std::make_unique<Shader>(
        "E:/2026-01/OpenGL/import-display-mesh/build/Debug/shaders/shader.vs",
        "E:/2026-01/OpenGL/import-display-mesh/build/Debug/shaders/shader.fs"
    );

    // Set initial background theme
    setBackgroundTheme(m_isDarkTheme);

    // Initialize grid shader
    m_gridShader = std::make_unique<Shader>(
        "E:/2026-01/OpenGL/import-display-mesh/build/Debug/shaders/grid.vs",
        "E:/2026-01/OpenGL/import-display-mesh/build/Debug/shaders/grid.fs"
    );
    setupGrid(); // Initial setup of the grid
}

void Viewer::setBackgroundTheme(bool isDarkTheme)
{
    m_isDarkTheme = isDarkTheme;
    if (background)
    {
        background->setDarkTheme(m_isDarkTheme);
    }
}

void Viewer::loadModel(const std::string &path)
{
    model = std::make_unique<Model>(path.c_str());

    // Setup model drop animation
    glm::vec3 modelCenter = model->GetCenter();
    glm::vec3 modelSize = model->GetSize();
    float maxDim = glm::max(glm::max(modelSize.x, modelSize.y), modelSize.z);

    // Calculate camera distance to fit the entire model in view (for immediate camera setup)
    float fovRadians = glm::radians(camera.GetZoom()); // Current camera FOV
    float distance = (maxDim / 2.0f) / glm::tan(fovRadians / 2.0f);
    distance *= 1.5f; // Add buffer

    // Immediately set camera to view the final resting position of the model
    glm::vec3 initialCameraPosition = modelCenter + glm::vec3(0.0f, 0.0f, distance); // Default front view
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);                                 // Assuming Y is up for the scene
    camera.SetPositionAndTarget(initialCameraPosition, modelCenter, worldUp);

    m_modelAnimEndPosition = modelCenter;
    // Start significantly above the model's actual center

    m_currentModelPosition = m_modelAnimEndPosition; // Start at the elevated position
    autoCenterAndOrientModel();

    // Adjust grid size based on model's max dimension
    m_gridExtent = static_cast<int>(maxDim * 5.0f / m_gridSpacing) + 1;
    setupGrid();
}

void Viewer::autoCenterAndOrientModel()
{
    if (!model)
    {
        return; // No model loaded, nothing to do
    }

    glm::vec3 modelCenter = model->GetCenter();
    glm::vec3 modelSize = model->GetSize();

    // Calculate camera distance to fit the entire model in view
    float maxDim = glm::max(glm::max(modelSize.x, modelSize.y), modelSize.z);
    float fovRadians = glm::radians(camera.GetZoom()); // This is FOV_y
    float distance = (maxDim / 2.0f) / glm::tan(fovRadians / 2.0f);

    // Add a buffer distance to ensure the model is fully visible
    distance *= 1.5f; // 50% buffer

    // Position the camera to look at the model's center from an isometric view (e.g., along (1.0, 1.0, 1.0) vector)
    glm::vec3 isometricDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    glm::vec3 targetCameraPosition = modelCenter + isometricDirection * distance;
    glm::vec3 targetWorldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Assuming Y is up for the scene

    m_cameraAnimator->startAnimation(camera, targetCameraPosition, modelCenter, targetWorldUp, camera.GetZoom(), CAMERA_ANIMATION_DURATION);
}

void Viewer::render()
{
    // Update delta time
    float currentFrameTime = static_cast<float>(glfwGetTime());
    m_deltaTime = currentFrameTime - m_lastFrameTime;
    m_lastFrameTime = currentFrameTime;

    // Handle camera animation
    if (m_cameraAnimator->isAnimating())
    {
        m_cameraAnimator->updateAnimation(m_deltaTime, camera);
    }

    // Determine the main rendering area
    int mainRenderWidth = width;
    int mainRenderHeight = height;
    int mainRenderX = 0;
    int mainRenderY = 0;

    if (m_showSummaryWindow)
    {
        mainRenderHeight -= static_cast<int>(m_summaryWindowHeight);
        mainRenderY = static_cast<int>(m_summaryWindowHeight); // The main view starts above the summary window
    }

    // Reset Viewport for main scene
    glViewport(mainRenderX, mainRenderY, mainRenderWidth, mainRenderHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Determine the aspect ratio to use
    float currentAspectRatio;
    if (m_useCustomAspectRatio)
    {
        currentAspectRatio = m_customAspectRatio;
    }
    else
    {
        currentAspectRatio = static_cast<float>(mainRenderWidth) / mainRenderHeight;
    }

    // 1. Draw Background
    if (background)
    {
        background->Draw();
    }

    // 2. Draw Grid
    drawGrid(mainRenderWidth, mainRenderHeight);

    // 2. Draw Model
    if (model && mainShader)
    {
        mainShader->use();
        // --- FIX START ---
        // You must strictly reset this to false, otherwise the "True" state
        // from the AxesWidget draw call (later in the frame) will leak into the NEXT frame.
        mainShader->setBool("u_isAxesWidget", false);
        // --- FIX END ---

        mainShader->setBool("u_lightingEnabled", m_lightingEnabled);  // Pass lighting toggle state
        mainShader->setFloat("u_ambientStrength", m_ambientStrength); // Pass ambient strength

        glm::mat4 projection;
        if (usePerspective)
            projection = glm::perspective(glm::radians(camera.GetZoom()), currentAspectRatio, 0.1f, 1000.0f);
        else
        {
            float orthoHeight = 2.0f * camera.GetRadius() * tan(glm::radians(camera.GetZoom()) / 2.0f);
            float orthoWidth = orthoHeight * currentAspectRatio;
            projection = glm::ortho(-orthoWidth / 2.0f, orthoWidth / 2.0f, -orthoHeight / 2.0f, orthoHeight / 2.0f, 0.1f, 1000.0f);
        }
        glm::mat4 view = camera.GetViewMatrix();
        mainShader->setMat4("projection", projection);
        mainShader->setMat4("view", view);

        // Apply model's animated position
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        mainShader->setMat4("model", modelMatrix);

        switch (m_modelViewMode)
        {
        case ModelViewMode::Shaded:
            glDisable(GL_BLEND);  // Ensure blending is off for opaque rendering
            glDepthMask(GL_TRUE); // Ensure depth writing is on
            mainShader->setBool("u_lightingEnabled", m_lightingEnabled);
            glPolygonOffset(1.0, 1.0);
            glEnable(GL_POLYGON_OFFSET_FILL);
            mainShader->setVec4("objectColor", glm::vec4(0.95f, 0.95f, 0.95f, 1.0f)); // Opaque white-ish
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            model->Draw();
            glDisable(GL_POLYGON_OFFSET_FILL);
            break;

        case ModelViewMode::Wireframe:
            // First, draw the solid model without writing to depth (so wireframe can be seen through it)
            glDepthMask(GL_FALSE);                                                    // Disable depth writing for the underlying solid model
            mainShader->setBool("u_lightingEnabled", m_lightingEnabled);              // Keep lighting for the underlying shaded part
            mainShader->setVec4("objectColor", glm::vec4(0.95f, 0.95f, 0.95f, 0.2f)); // Semi-transparent shaded part
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_BLEND); // Enable blending for transparency
            model->Draw();

            // Then, draw the wireframe on top
            glDepthMask(GL_TRUE);                                                  // Enable depth writing for the wireframe (so it doesn't draw over everything)
            mainShader->setBool("u_lightingEnabled", false);                       // No lighting for wireframe
            mainShader->setVec4("objectColor", glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)); // Opaque blue wireframe on top
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0f); // Default wireframe thickness
            model->Draw();

            glDisable(GL_BLEND);                       // Disable blending after drawing transparent objects
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset polygon mode
            break;

        case ModelViewMode::ShadedWithEdges:
            glDisable(GL_BLEND);  // Ensure blending is off for opaque rendering
            glDepthMask(GL_TRUE); // Ensure depth writing is on
            // Draw solid shaded model
            mainShader->setBool("u_lightingEnabled", m_lightingEnabled);
            glPolygonOffset(1.0, 1.0);
            glEnable(GL_POLYGON_OFFSET_FILL);
            mainShader->setVec4("objectColor", glm::vec4(0.95f, 0.95f, 0.95f, 1.0f)); // Opaque white-ish
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            model->Draw();
            glDisable(GL_POLYGON_OFFSET_FILL);

            // Draw wireframe overlay
            mainShader->setBool("u_lightingEnabled", false);                       // No lighting for wireframe overlay
            mainShader->setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red edges
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(0.5f); // Thinner edges
            model->Draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                   // Reset to fill for next draw calls
            mainShader->setBool("u_lightingEnabled", m_lightingEnabled); // Restore lighting state
            break;
        }

        // 3. Draw Axes Widget
        if (axesWidget && m_showAxesWidget)
        {
            axesWidget->Draw(view, projection, *mainShader, mainRenderX, mainRenderY, mainRenderWidth, mainRenderHeight); // Pass screen width and height
        }

        // Setup Dockspace
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        // 4. ImGui: Summary Information Window
        if (m_showSummaryWindow)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowViewport()->Pos.x, ImGui::GetWindowViewport()->Pos.y + ImGui::GetWindowViewport()->Size.y - m_summaryWindowHeight), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowViewport()->Size.x, m_summaryWindowHeight), ImGuiCond_Always);
            ImGui::Begin("Summary Information", &m_showSummaryWindow, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

            ImGui::Text("View Settings:");
            ImGui::SameLine();
            switch (m_modelViewMode)
            {
            case ModelViewMode::Shaded:
                ImGui::Text("Mode: Shaded");
                break;
            case ModelViewMode::Wireframe:
                ImGui::Text("Mode: Wireframe");
                break;
            case ModelViewMode::ShadedWithEdges:
                ImGui::Text("Mode: Shaded With Edges");
                break;
            }
            ImGui::SameLine();
            ImGui::Text("| Projection: %s", usePerspective ? "Perspective" : "Orthographic");

            if (model)
            {
                ImGui::Text("Geometry Info:");
                ImGui::SameLine();
                ImGui::Text("Center: (%.2f, %.2f, %.2f)", model->GetCenter().x, model->GetCenter().y, model->GetCenter().z);
                ImGui::SameLine();
                ImGui::Text("Size: (%.2f, %.2f, %.2f)", model->GetSize().x, model->GetSize().y, model->GetSize().z);
                ImGui::SameLine();
                ImGui::Text("Min Bounds: (%.2f, %.2f, %.2f)", model->GetBoundingBoxMin().x, model->GetBoundingBoxMin().y, model->GetBoundingBoxMin().z);
                ImGui::SameLine();
                ImGui::Text("Max Bounds: (%.2f, %.2f, %.2f)", model->GetBoundingBoxMax().x, model->GetBoundingBoxMax().y, model->GetBoundingBoxMax().z);
            }
            else
            {
                ImGui::Text("No model loaded.");
            }
            ImGui::End();
        }

        // 5. ImGui: Context Menu (replaces old custom context menu logic)
        // Use BeginPopupContextVoid to create a global popup not tied to any specific ImGui window.
        if (ImGui::BeginPopupContextVoid("ModelViewerContextMenu", ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Toggle Lighting (L)", nullptr, m_lightingEnabled))
            {
                m_lightingEnabled = !m_lightingEnabled;
            }
            if (ImGui::BeginMenu("View Mode (V)"))
            {
                if (ImGui::MenuItem("Shaded", nullptr, m_modelViewMode == ModelViewMode::Shaded))
                {
                    m_modelViewMode = ModelViewMode::Shaded;
                }
                if (ImGui::MenuItem("Wireframe", nullptr, m_modelViewMode == ModelViewMode::Wireframe))
                {
                    m_modelViewMode = ModelViewMode::Wireframe;
                }
                if (ImGui::MenuItem("Shaded With Edges", nullptr, m_modelViewMode == ModelViewMode::ShadedWithEdges))
                {
                    m_modelViewMode = ModelViewMode::ShadedWithEdges;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Toggle Perspective (P)", nullptr, usePerspective))
            {
                usePerspective = !usePerspective;
            }
            if (ImGui::BeginMenu("Theme"))
            {
                if (ImGui::MenuItem("Default", nullptr, g_currentThemeOption == ThemeOption::Default))
                {
                    setCurrentTheme(ThemeOption::Default);
                    setBackgroundTheme(g_isDarkMode);
                }
                if (ImGui::MenuItem("Light", nullptr, g_currentThemeOption == ThemeOption::Light))
                {
                    setCurrentTheme(ThemeOption::Light);
                    setBackgroundTheme(g_isDarkMode);
                }
                if (ImGui::MenuItem("Dark", nullptr, g_currentThemeOption == ThemeOption::Dark))
                {
                    setCurrentTheme(ThemeOption::Dark);
                    setBackgroundTheme(g_isDarkMode);
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Toggle Axes (C)", nullptr, m_showAxesWidget))
            {
                m_showAxesWidget = !m_showAxesWidget;
            }
            if (ImGui::MenuItem("Toggle Grid", nullptr, m_showGrid))
            {
                m_showGrid = !m_showGrid;
            }
            if (ImGui::MenuItem("Auto-center Model (A)"))
            {
                autoCenterAndOrientModel();
            }
            if (ImGui::MenuItem("Import Model (I)"))
            {
                char const *lTheOpenFileName;
                char const *lFilterPatterns[2] = {"*.obj", "*.stl"};
                lTheOpenFileName = tinyfd_openFileDialog(
                    "Open 3D Model", "", 2, lFilterPatterns, "3D Model Files (*.obj, *.stl)", 0);
                if (lTheOpenFileName)
                {
                    std::string filePath(lTheOpenFileName);
                    loadModel(filePath);
                }
                else
                {
                    std::cout << "No file selected." << std::endl;
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle Debug Window (D)", nullptr, m_showDebugWindow))
            {
                m_showDebugWindow = !m_showDebugWindow;
            }
            if (ImGui::MenuItem("Toggle Camera Window (T)", nullptr, m_showCameraWindow))
            {
                m_showCameraWindow = !m_showCameraWindow;
            }
            if (ImGui::MenuItem("Toggle Grid Control Window", nullptr, m_showGridControlWindow))
            {
                m_showGridControlWindow = !m_showGridControlWindow;
            }
            if (ImGui::MenuItem("Toggle Summary Window", nullptr, m_showSummaryWindow))
            {
                m_showSummaryWindow = !m_showSummaryWindow;
            }
            ImGui::EndPopup();
        }

        // 5. ImGui: Debug Window
        if (m_showDebugWindow)
        {
            ImGui::SetNextWindowPos(ImGui::GetWindowViewport()->Pos, ImGuiCond_Always); // Top-left of the application window
            ImGui::Begin("Camera Debug Info", &m_showDebugWindow);
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
            ImGui::Text("Target:   (%.2f, %.2f, %.2f)", camera.GetTarget().x, camera.GetTarget().y, camera.GetTarget().z);
            ImGui::Text("Up:       (%.2f, %.2f, %.2f)", camera.GetUp().x, camera.GetUp().y, camera.GetUp().z);
            ImGui::Text("Front:    (%.2f, %.2f, %.2f)", camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
            ImGui::Text("Right:    (%.2f, %.2f, %.2f)", camera.GetRight().x, camera.GetRight().y, camera.GetRight().z);
            ImGui::Text("Radius:   %.2f", camera.GetRadius());

            ImGui::End();
        }

        // 6. ImGui: Camera Control Window
        if (m_showCameraWindow)
        {
            // Position top-right of the application window's viewport
            float posX = ImGui::GetWindowViewport()->Pos.x + ImGui::GetWindowViewport()->Size.x;
            ImGui::SetNextWindowPos(ImVec2(posX, ImGui::GetWindowViewport()->Pos.y), ImGuiCond_Always, ImVec2(1, 0));
            ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_FirstUseEver); // Still allow size to be remembered
            ImGui::Begin("Camera Control", &m_showCameraWindow);

            float currentFov = camera.GetZoom(); // Initialize with current camera FOV
            bool valueEdited = false;            // Flag to track if any value was edited

            // Sensor Height (mm)
            ImGui::AlignTextToFramePadding(); // Align text vertically with the following widget
            ImGui::Text("Sensor Height (mm)");
            ImGui::SameLine();
            float itemWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::InputFloat("##SensorHeight", &m_sensorHeight, 1.0f, 10.0f, "%.1f"))
            {
                valueEdited = true;
                m_sensorHeight = glm::max(0.1f, m_sensorHeight); // Ensure sensor height is not too small
                // Recalculate focal length based on current FOV and new sensor height
                float fovy_radians = glm::radians(currentFov);
                m_focalLength = (m_sensorHeight / 2.0f) / glm::tan(fovy_radians / 2.0f);
            }
            ImGui::PopItemWidth();

            // FOV (degrees)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("FOV (degrees)");
            ImGui::SameLine();
            itemWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::SliderFloat("##FOV", &currentFov, 1.0f, 120.0f))
            {
                valueEdited = true;
                currentFov = glm::clamp(currentFov, 0.1f, 179.9f); // Clamp FOV for glm::perspective stability
                camera.SetZoom(currentFov);                        // Update camera's FOV
                // Recalculate Focal Length based on new FOV and current sensor height
                float fovy_radians = glm::radians(currentFov);
                m_focalLength = (m_sensorHeight / 2.0f) / glm::tan(fovy_radians / 2.0f);
            }
            ImGui::PopItemWidth();

            // Focal Length (mm)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Focal Length (mm)");
            ImGui::SameLine();
            itemWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::SliderFloat("##FocalLength", &m_focalLength, 1.0f, 200.0f, "%.1f")) // Lower min focal length
            {
                valueEdited = true;
                m_focalLength = glm::max(0.1f, m_focalLength); // Ensure focal length is not too small
                // Recalculate FOV based on new Focal Length and current sensor height
                float fovy_radians = 2.0f * glm::atan((m_sensorHeight / 2.0f) / m_focalLength);
                camera.SetZoom(glm::degrees(fovy_radians)); // Update camera's FOV
            }
            ImGui::PopItemWidth();

            ImGui::Separator();

            // Toggle for Custom Aspect Ratio
            bool prevUseCustomAspectRatio = m_useCustomAspectRatio;
            ImGui::Checkbox("Use Custom Aspect Ratio", &m_useCustomAspectRatio);
            if (!prevUseCustomAspectRatio && m_useCustomAspectRatio)
            {
                // If custom AR was just enabled, set m_customAspectRatio to the current EFFECTIVE rendering area AR
                int effectiveHeight = height;
                if (m_showSummaryWindow)
                {
                    effectiveHeight -= static_cast<int>(m_summaryWindowHeight);
                }
                if (effectiveHeight > 0)
                { // Avoid division by zero
                    m_customAspectRatio = static_cast<float>(width) / effectiveHeight;
                }
                else
                {
                    m_customAspectRatio = 1.0f; // Default to 1:1 if effective height is zero or less
                }
            }

            // Aspect Ratio slider, only editable when m_useCustomAspectRatio is true
            float currentSliderAspectRatio = m_customAspectRatio;
            if (!m_useCustomAspectRatio)
            {
                // Calculate the effective aspect ratio for display on the slider
                // when custom aspect ratio is not in use.
                // This should reflect the aspect ratio of the actual rendering viewport.
                currentSliderAspectRatio = static_cast<float>(mainRenderWidth) / mainRenderHeight;
            }

            ImGui::BeginDisabled(!m_useCustomAspectRatio); // Disable slider if not using custom AR
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Aspect Ratio");
            ImGui::SameLine();
            itemWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::SliderFloat("##CustomAspectRatio", &currentSliderAspectRatio, 0.2f, 3.0f, "%.2f"))
            {
                // Only update m_customAspectRatio if the slider is enabled (i.e., m_useCustomAspectRatio is true)
                m_customAspectRatio = currentSliderAspectRatio;
            }
            ImGui::PopItemWidth();
            m_customAspectRatio = glm::max(0.01f, m_customAspectRatio); // Ensure aspect ratio is not too small
            ImGui::EndDisabled();                                       // End disable block

            if (ImGui::Button("Reset View"))
            {
                autoCenterAndOrientModel();
            }

            ImGui::End();
        }

        // 7. ImGui: Grid Control Window
        if (m_showGridControlWindow)
        {
            float posX = ImGui::GetWindowViewport()->Pos.x + ImGui::GetWindowViewport()->Size.x;
            ImGui::SetNextWindowPos(ImVec2(posX, ImGui::GetWindowViewport()->Pos.y + ImGui::GetWindowViewport()->Size.y / 2.0f), ImGuiCond_Always, ImVec2(1, 0.5));
            ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_FirstUseEver);
            ImGui::Begin("Grid Control", &m_showGridControlWindow);

            bool gridSettingsChanged = false;

            // Grid Spacing
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Grid Spacing");
            ImGui::SameLine();
            float itemWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::SliderFloat("##GridSpacing", &m_gridSpacing, 0.1f, 10.0f, "%.1f"))
            {
                gridSettingsChanged = true;
            }
            ImGui::PopItemWidth();
            m_gridSpacing = glm::max(0.01f, m_gridSpacing); // Ensure spacing is not too small

            // Grid Extent
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Grid Extent");
            ImGui::SameLine();
            itemWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushItemWidth(itemWidth);
            if (ImGui::SliderInt("##GridExtent", &m_gridExtent, 1, 100))
            {
                gridSettingsChanged = true;
            }
            ImGui::PopItemWidth();
            m_gridExtent = glm::max(1, m_gridExtent); // Ensure extent is at least 1

            // Grid Color
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Grid Color");
            ImGui::SameLine();
            ImGui::ColorEdit4("##GridColor", glm::value_ptr(m_gridColor));

            if (gridSettingsChanged)
            {
                setupGrid(); // Regenerate grid with new settings
            }

            ImGui::End();
        }
    }
}

void Viewer::OnAxesWidgetClick(Axis clickedAxis)
{
    std::cout << "OnAxesWidgetClick: Initial clicked axis: " << static_cast<int>(clickedAxis) << std::endl;

    glm::vec3 modelCenter = glm::vec3(0.0f);
    float distance = camera.GetRadius();

    if (model)
    {
        modelCenter = model->GetCenter();
        glm::vec3 modelSize = model->GetSize();
        float maxDim = glm::max(glm::max(modelSize.x, modelSize.y), modelSize.z);
        float fovRadians = glm::radians(camera.GetZoom());
        distance = (maxDim / 2.0f) / glm::tan(fovRadians / 2.0f);
        distance *= 1.5f; // Add buffer
    }

    Axis currentViewingAxis = camera.GetCurrentViewingAxis();
    std::cout << "OnAxesWidgetClick: Current viewing axis from camera: " << static_cast<int>(currentViewingAxis) << std::endl;

    Axis targetAxis;
    Axis clickedCardinal = getCardinalAxisEnum(clickedAxis);
    Axis lastAlignedCardinal = getCardinalAxisEnum(m_lastAlignedAxis); // Get cardinal from stored last aligned axis

    std::cout << "OnAxesWidgetClick: Clicked cardinal axis: " << static_cast<int>(clickedCardinal) << std::endl;
    std::cout << "OnAxesWidgetClick: Last aligned cardinal axis: " << static_cast<int>(lastAlignedCardinal) << std::endl;

    // Use m_lastAlignedAxis to determine if we should flip
    if (m_lastAlignedAxis != Axis::NONE && clickedCardinal == lastAlignedCardinal)
    {
        // If the clicked axis's cardinal direction matches the last intended aligned axis, then flip.
        targetAxis = GetAxisOpposite(m_lastAlignedAxis); // Flip the last intended aligned axis
        std::cout << "OnAxesWidgetClick: Decision: Flipped to opposite view based on last aligned. Target: " << static_cast<int>(targetAxis) << std::endl;
    }
    else
    {
        // If no last aligned axis, or clicked axis is for a different cardinal direction,
        // just orient to the clicked axis.
        targetAxis = clickedAxis;
        std::cout << "OnAxesWidgetClick: Decision: Set to clicked axis. Target: " << static_cast<int>(targetAxis) << std::endl;
    }
    m_lastAlignedAxis = targetAxis; // Always update m_lastAlignedAxis after determining targetAxis

    glm::vec3 targetCameraPosition;
    glm::vec3 targetWorldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Default targetWorldUp for most views is World Y-up

    switch (targetAxis) // Use targetAxis here
    {
    case Axis::X_POS:
        targetCameraPosition = modelCenter + glm::vec3(distance, 0.0f, 0.0f);
        break;
    case Axis::X_NEG:
        targetCameraPosition = modelCenter + glm::vec3(-distance, 0.0f, 0.0f);
        break;
    case Axis::Y_POS: // Top View
        targetCameraPosition = modelCenter + glm::vec3(0.0f, distance, 0.0f);
        targetWorldUp = glm::vec3(0.0f, 0.0f, 1.0f); // X-right, +Z up (for Y-up world)
        break;
    case Axis::Y_NEG: // Bottom View
        targetCameraPosition = modelCenter + glm::vec3(0.0f, -distance, 0.0f);
        targetWorldUp = glm::vec3(0.0f, 0.0f, 1.0f); // X-right, -Z up (for Y-up world)
        break;
    case Axis::Z_POS:
        targetCameraPosition = modelCenter + glm::vec3(0.0f, 0.0f, distance);
        break;
    case Axis::Z_NEG:
        targetCameraPosition = modelCenter + glm::vec3(0.0f, 0.0f, -distance);
        break;
    case Axis::NONE: // Should ideally not be triggered by a click on an axis label
        std::cout << "OnAxesWidgetClick: Axis::NONE received for targetAxis, returning." << std::endl;
        return;
    }

    std::cout << "OnAxesWidgetClick: Final Target Camera Position: (" << targetCameraPosition.x << ", " << targetCameraPosition.y << ", " << targetCameraPosition.z << ")" << std::endl;
    std::cout << "OnAxesWidgetClick: Final Target World Up: (" << targetWorldUp.x << ", " << targetWorldUp.y << ", " << targetWorldUp.z << ")" << std::endl;

    m_cameraAnimator->startAnimation(camera, targetCameraPosition, modelCenter, targetWorldUp, camera.GetZoom(), CAMERA_ANIMATION_DURATION);
}

void Viewer::onResize(int w, int h)
{
    width = w;
    height = h;

    m_lastWindowAspectRatio = static_cast<float>(width) / height;
    // Update TextRenderer and AxesWidget with new screen dimensions
    // Re-initialize unique_ptrs to recreate objects with new dimensions
    textRenderer = std::make_unique<TextRenderer>(width, height);
    axesWidget = std::make_unique<AxesWidget>(width, height, uiRenderer.get(), this);
}

void Viewer::onKey(int key, int action)
{
    // Key bindings are now handled by context menu
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        usePerspective = !usePerspective;
        // std::cout << "P key pressed. Perspective Mode Enabled: " << (usePerspective ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        switch (m_modelViewMode)
        {
        case ModelViewMode::Shaded:
            m_modelViewMode = ModelViewMode::Wireframe;
            // std::cout << "V key pressed. Model View Mode: Wireframe" << std::endl;
            break;
        case ModelViewMode::Wireframe:
            m_modelViewMode = ModelViewMode::ShadedWithEdges;
            // std::cout << "V key pressed. Model View Mode: Shaded with Feature Edges" << std::endl;
            break;
        case ModelViewMode::ShadedWithEdges:
            m_modelViewMode = ModelViewMode::Shaded;
            // std::cout << "V key pressed. Model View Mode: Shaded" << std::endl;
            break;
        }
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        m_lightingEnabled = !m_lightingEnabled; // Toggle lighting with 'L' key
        // std::cout << "L key pressed. Lighting Enabled: " << (m_lightingEnabled ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        m_showAxesWidget = !m_showAxesWidget;
        // std::cout << "C key pressed. Axes Widget Visible: " << (m_showAxesWidget ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) // New: Auto-center and orient with 'A' key
    {
        autoCenterAndOrientModel();
        // std::cout << "A key pressed. Auto-centered and oriented model." << std::endl;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        ThemeOption newTheme = g_isDarkMode ? ThemeOption::Light : ThemeOption::Dark;
        setCurrentTheme(newTheme);
        setBackgroundTheme(g_isDarkMode);
    }

    if (key == GLFW_KEY_I && action == GLFW_PRESS) // New: Open file dialog to import model
    {
        char const *lTheOpenFileName;
        char const *lFilterPatterns[2] = {"*.obj", "*.stl"}; // Filter for .obj and .stl files

        lTheOpenFileName = tinyfd_openFileDialog(
            "Open 3D Model",
            "", // Default path: empty, opens in current directory or last used
            2,  // Number of filter patterns
            lFilterPatterns,
            "3D Model Files (*.obj, *.stl)",
            0 // Allow multiple selections: 0 for single, 1 for multiple
        );

        if (lTheOpenFileName) // If a file was selected
        {
            std::string filePath(lTheOpenFileName);
            loadModel(filePath); // Load the selected model
            // std::cout << "I key pressed. Loaded model: " << filePath << std::endl;
        }
        else
        {
            std::cout << "I key pressed. No file selected." << std::endl;
        }
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        m_showDebugWindow = !m_showDebugWindow;
        // std::cout << "D key pressed. Debug Window Visible: " << (m_showDebugWindow ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        m_showCameraWindow = !m_showCameraWindow;
    }
}

void Viewer::onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton)
{
    // If ImGui is capturing the mouse, do not process camera movement
    if (ImGui::GetIO().WantCaptureMouse)
    {
        firstMouse = true; // Reset firstMouse to avoid jump when orbiting after ImGui interaction
        return;
    }

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (leftButton)
    {
        camera.ProcessMouseOrbit(xoffset, yoffset);
    }
    if (middleButton)
    {
        camera.ProcessMousePanning(xoffset, yoffset, height);
    }
}

void Viewer::onScroll(float yoffset)
{
    // If ImGui is capturing the mouse, do not process camera scroll
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    camera.ProcessMouseScroll(yoffset);
}

void Viewer::onMouseButton(int button, int action, double xpos, double ypos)
{
    // If ImGui is capturing the mouse, do not process camera related mouse button events
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    // Convert mouse ypos to OpenGL's bottom-up coordinate system for AxesWidget
    double opengl_ypos = height - ypos;

    // Check if AxesWidget handled the mouse button event
    if (axesWidget && m_showAxesWidget)
    {
        if (axesWidget->OnMouseButton(xpos, opengl_ypos, button, action))
        {
            return; // Event handled by AxesWidget, do not process further
        }
    }

    // Original mouse button handling for camera.
}

void Viewer::drawGrid(int viewportWidth, int viewportHeight)
{
    if (!m_gridShader || !m_showGrid)
    {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Draw grid always on top or at a fixed depth

    m_gridShader->use();

    glm::mat4 projection;
    float currentAspectRatio;
    if (m_useCustomAspectRatio)
    {
        currentAspectRatio = m_customAspectRatio;
    }
    else
    {
        currentAspectRatio = static_cast<float>(viewportWidth) / viewportHeight;
    }

    if (usePerspective)
        projection = glm::perspective(glm::radians(camera.GetZoom()), currentAspectRatio, 0.1f, 1000.0f);
    else
    {
        float orthoHeight = 2.0f * camera.GetRadius() * tan(glm::radians(camera.GetZoom()) / 2.0f);
        float orthoWidth = orthoHeight * currentAspectRatio;
        projection = glm::ortho(-orthoWidth / 2.0f, orthoWidth / 2.0f, -orthoHeight / 2.0f, orthoHeight / 2.0f, 0.1f, 1000.0f);
    }
    glm::mat4 view = camera.GetViewMatrix();

    m_gridShader->setMat4("projection", projection);
    m_gridShader->setMat4("view", view);

    // Draw X-axis (Red)
    m_gridShader->setVec4("gridColor", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f)); // X Axis (Red-ish)
    glBindVertexArray(m_xAxisVAO);
    glDrawArrays(GL_LINES, 0, 2); // 2 vertices for one line
    glBindVertexArray(0);

    // Draw Y-axis (Green)
    m_gridShader->setVec4("gridColor", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f)); // Y Axis (Green-ish)
    glBindVertexArray(m_yAxisVAO);
    glDrawArrays(GL_LINES, 0, 2); // 2 vertices for one line
    glBindVertexArray(0);

    // Draw major grid lines (half transparent)
    m_gridShader->setVec4("gridColor", glm::vec4(m_gridColor.r, m_gridColor.g, m_gridColor.b, 0.5f)); // Half transparent
    glBindVertexArray(m_majorGridVAO);
    int numMajorGridVertices = (m_gridExtent * 2) * 2 + (m_gridExtent * 2) * 2; // (Number of lines in X-dir * 2 vertices) + (Number of lines in Y-dir * 2 vertices)
    glDrawArrays(GL_LINES, 0, numMajorGridVertices);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST); // Re-enable depth test for other objects
    glDisable(GL_BLEND);     // Disable blending
}

void Viewer::setupGrid()
{
    // Delete existing VAO/VBOs if they exist
    if (m_xAxisVAO != 0)
        glDeleteVertexArrays(1, &m_xAxisVAO);
    if (m_xAxisVBO != 0)
        glDeleteBuffers(1, &m_xAxisVBO);
    if (m_yAxisVAO != 0)
        glDeleteVertexArrays(1, &m_yAxisVAO);
    if (m_yAxisVBO != 0)
        glDeleteBuffers(1, &m_yAxisVBO);
    if (m_majorGridVAO != 0)
        glDeleteVertexArrays(1, &m_majorGridVAO);
    if (m_majorGridVBO != 0)
        glDeleteBuffers(1, &m_majorGridVBO);

    float maxCoord = m_gridExtent * m_gridSpacing;

    // --- X-Axis (Red) ---
    std::vector<glm::vec3> xAxisVertices;
    xAxisVertices.push_back(glm::vec3(-maxCoord, 0.0f, 0.0f));
    xAxisVertices.push_back(glm::vec3(maxCoord, 0.0f, 0.0f));
    glGenVertexArrays(1, &m_xAxisVAO);
    glGenBuffers(1, &m_xAxisVBO);
    glBindVertexArray(m_xAxisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_xAxisVBO);
    glBufferData(GL_ARRAY_BUFFER, xAxisVertices.size() * sizeof(glm::vec3), xAxisVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- Y-Axis (Green) ---
    std::vector<glm::vec3> yAxisVertices;
    yAxisVertices.push_back(glm::vec3(0.0f, -maxCoord, 0.0f));
    yAxisVertices.push_back(glm::vec3(0.0f, maxCoord, 0.0f));
    glGenVertexArrays(1, &m_yAxisVAO);
    glGenBuffers(1, &m_yAxisVBO);
    glBindVertexArray(m_yAxisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_yAxisVBO);
    glBufferData(GL_ARRAY_BUFFER, yAxisVertices.size() * sizeof(glm::vec3), yAxisVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- Major Grid Lines (excluding axes) ---
    std::vector<glm::vec3> majorGridVertices;
    // Generate X-parallel lines
    for (int i = -m_gridExtent; i <= m_gridExtent; ++i)
    {
        if (i == 0)
            continue; // Skip X-axis (y=0) as it's handled separately
        float y = i * m_gridSpacing;
        majorGridVertices.push_back(glm::vec3(-maxCoord, y, 0.0f));
        majorGridVertices.push_back(glm::vec3(maxCoord, y, 0.0f));
    }

    // Generate Y-parallel lines
    for (int i = -m_gridExtent; i <= m_gridExtent; ++i)
    {
        if (i == 0)
            continue; // Skip Y-axis (x=0) as it's handled separately
        float x = i * m_gridSpacing;
        majorGridVertices.push_back(glm::vec3(x, -maxCoord, 0.0f));
        majorGridVertices.push_back(glm::vec3(x, maxCoord, 0.0f));
    }

    glGenVertexArrays(1, &m_majorGridVAO);
    glGenBuffers(1, &m_majorGridVBO);
    glBindVertexArray(m_majorGridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_majorGridVBO);
    glBufferData(GL_ARRAY_BUFFER, majorGridVertices.size() * sizeof(glm::vec3), majorGridVertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}