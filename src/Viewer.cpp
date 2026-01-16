#include "Viewer.h"
#include "Background.h"
#include "AxesWidget.h"
#include "TextRenderer.h" // Include TextRenderer
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <tinyfiledialogs.h>

Viewer::Viewer(int width, int height)
    : width(width), height(height),
      camera(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      usePerspective(false), firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f),
      m_lightingEnabled(true), m_ambientStrength(0.1f), // Initialize new members
      m_lastFrameTime(0.0f),                            // Initialize m_lastFrameTime here
      m_currentModelPosition(0.0f, 0.0f, 0.0f),
      m_isDarkTheme(false) // Initialize m_isDarkTheme
{
    background = std::make_unique<Background>();
    uiRenderer = std::make_unique<UiRenderer>();                                // Initialize UiRenderer
    axesWidget = std::make_unique<AxesWidget>(width, height, uiRenderer.get()); // Initialize AxesWidget with screen dimensions and uiRenderer
    textRenderer = std::make_unique<TextRenderer>(width, height);               // Initialize TextRenderer
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

    // Assume shaders are relative to CWD
    mainShader = std::make_unique<Shader>("shaders/shader.vs", "shaders/shader.fs");

    // Set initial background theme
    setBackgroundTheme(m_isDarkTheme);
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
    m_modelAnimStartPosition = modelCenter + glm::vec3(0.0f, maxDim * 10.0f, 0.0f); // 10 times maxDim above

    m_isAnimatingModelDrop = true;
    m_modelDropTime = 0.0f;
    m_modelDropDuration = 2.0f;                        // Increase duration to 2 seconds
    m_currentModelPosition = m_modelAnimStartPosition; // Start at the elevated position
}

void Viewer::autoCenterAndOrientModel()
{
    if (!model)
    {
        return; // No model loaded, nothing to do
    }

    // Store current camera state as animation start
    m_cameraAnimStartPosition = camera.GetPosition();
    m_cameraAnimStartTarget = camera.GetTarget();
    m_cameraAnimStartWorldUp = camera.GetUp();

    glm::vec3 modelCenter = model->GetCenter();
    glm::vec3 modelSize = model->GetSize();

    // Calculate camera distance to fit the entire model in view
    float maxDim = glm::max(glm::max(modelSize.x, modelSize.y), modelSize.z);
    float fovRadians = glm::radians(camera.GetZoom()); // This is FOV_y
    float distance = (maxDim / 2.0f) / glm::tan(fovRadians / 2.0f);

    // Add a buffer distance to ensure the model is fully visible
    distance *= 1.5f; // 50% buffer

    // Position the camera to look at the model's center from an isometric view (e.g., along (1,1,1) vector)
    glm::vec3 isometricDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    glm::vec3 targetCameraPosition = modelCenter + isometricDirection * distance;
    glm::vec3 targetWorldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Assuming Y is up for the scene

    // Store target camera state as animation end
    m_cameraAnimEndPosition = targetCameraPosition;
    m_cameraAnimEndTarget = modelCenter;
    m_cameraAnimEndWorldUp = targetWorldUp;

    // Start animation
    m_isAnimatingCamera = true;
    m_animationTime = 0.0f;
}

void Viewer::render()
{
    // Update delta time
    float currentFrameTime = static_cast<float>(glfwGetTime());
    m_deltaTime = currentFrameTime - m_lastFrameTime;
    m_lastFrameTime = currentFrameTime;

    // Handle camera animation
    if (m_isAnimatingCamera)
    {
        m_animationTime += m_deltaTime;
        float t = glm::clamp(m_animationTime / m_animationDuration, 0.0f, 1.0f); // Animation progress [0, 1]

        // Use smoothstep for smoother animation (optional, linear is fine too)
        // t = t * t * (3.0f - 2.0f * t);

        glm::vec3 currentPosition = glm::mix(m_cameraAnimStartPosition, m_cameraAnimEndPosition, t);
        glm::vec3 currentTarget = glm::mix(m_cameraAnimStartTarget, m_cameraAnimEndTarget, t);
        // For 'Up' vector, usually Slerp for quaternions or mix for vectors if they are already normalized
        glm::vec3 currentWorldUp = glm::mix(m_cameraAnimStartWorldUp, m_cameraAnimEndWorldUp, t);
        currentWorldUp = glm::normalize(currentWorldUp); // Ensure it stays normalized

        camera.SetPositionAndTarget(currentPosition, currentTarget, currentWorldUp);

        if (m_animationTime >= m_animationDuration)
        {
            m_isAnimatingCamera = false;
            // Ensure camera is precisely at the end state
            camera.SetPositionAndTarget(m_cameraAnimEndPosition, m_cameraAnimEndTarget, m_cameraAnimEndWorldUp);
        }
    }

    // Handle model drop animation
    if (m_isAnimatingModelDrop)
    {
        m_modelDropTime += m_deltaTime;
        float t = glm::clamp(m_modelDropTime / m_modelDropDuration, 0.0f, 1.0f);

        // Smoothstep for model drop (optional)
        t = t * t * (3.0f - 2.0f * t);

        m_currentModelPosition = glm::mix(m_modelAnimStartPosition, m_modelAnimEndPosition, t);

        if (m_modelDropTime >= m_modelDropDuration)
        {
            m_isAnimatingModelDrop = false;
            m_currentModelPosition = m_modelAnimEndPosition; // Ensure it lands precisely
            autoCenterAndOrientModel();                      // Trigger camera centering after model drop
        }
    }
    // Reset Viewport for main scene
    glViewport(0, 0, width, height);
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Removed as background is drawn by Background class
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspectRatio = (float)width / (float)height;

    // 1. Draw Background
    if (background)
    {
        background->Draw();
    }

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
            projection = glm::perspective(glm::radians(camera.GetZoom()), aspectRatio, 0.1f, 1000.0f);
        else
        {
            float orthoHeight = 2.0f * camera.GetRadius() * tan(glm::radians(camera.GetZoom()) / 2.0f);
            float orthoWidth = orthoHeight * aspectRatio;
            projection = glm::ortho(-orthoWidth / 2.0f, orthoWidth / 2.0f, -orthoHeight / 2.0f, orthoHeight / 2.0f, 0.1f, 1000.0f);
        }
        glm::mat4 view = camera.GetViewMatrix();
        mainShader->setMat4("projection", projection);
        mainShader->setMat4("view", view);

        // Apply model's animated position
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), m_currentModelPosition - m_modelAnimEndPosition);
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
            axesWidget->Draw(view, projection, *mainShader, width, height); // Pass screen width and height
        }

        // 4. Draw Context Menu if active
        if (m_showContextMenu)
        {
            drawContextMenu();
        }
    }
}

void Viewer::onResize(int w, int h)
{
    width = w;
    height = h;
    // Update TextRenderer and AxesWidget with new screen dimensions
    // Re-initialize unique_ptrs to recreate objects with new dimensions
    textRenderer = std::make_unique<TextRenderer>(width, height);
    axesWidget = std::make_unique<AxesWidget>(width, height, uiRenderer.get());
}

void Viewer::onKey(int key, int action)
{
    // Key bindings are now handled by context menu
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        usePerspective = !usePerspective;
        std::cout << "P key pressed. Perspective Mode Enabled: " << (usePerspective ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        switch (m_modelViewMode)
        {
        case ModelViewMode::Shaded:
            m_modelViewMode = ModelViewMode::Wireframe;
            std::cout << "V key pressed. Model View Mode: Wireframe" << std::endl;
            break;
        case ModelViewMode::Wireframe:
            m_modelViewMode = ModelViewMode::ShadedWithEdges;
            std::cout << "V key pressed. Model View Mode: Shaded with Feature Edges" << std::endl;
            break;
        case ModelViewMode::ShadedWithEdges:
            m_modelViewMode = ModelViewMode::Shaded;
            std::cout << "V key pressed. Model View Mode: Shaded" << std::endl;
            break;
        }
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        m_lightingEnabled = !m_lightingEnabled; // Toggle lighting with 'L' key
        std::cout << "L key pressed. Lighting Enabled: " << (m_lightingEnabled ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        m_showAxesWidget = !m_showAxesWidget;
        std::cout << "C key pressed. Axes Widget Visible: " << (m_showAxesWidget ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) // New: Auto-center and orient with 'A' key
    {
        autoCenterAndOrientModel();
        std::cout << "A key pressed. Auto-centered and oriented model." << std::endl;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) // New: Toggle background theme with 'B' key
    {
        setBackgroundTheme(!m_isDarkTheme);
        std::cout << "B key pressed. Dark theme enabled: " << (m_isDarkTheme ? "true" : "false") << std::endl;
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
            std::cout << "I key pressed. Loaded model: " << filePath << std::endl;
        }
        else
        {
            std::cout << "I key pressed. No file selected." << std::endl;
        }
    }
}

void Viewer::onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton)
{
    if (m_showContextMenu)
        return; // Do not orbit if context menu is open

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
    if (m_showContextMenu)
        return; // Do not zoom if context menu is open
    camera.ProcessMouseScroll(yoffset);
}

void Viewer::drawContextMenu()
{
    if (!uiRenderer || !textRenderer)
        return;

    // Set up orthographic projection for 2D UI
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

    // Calculate menu height based on the number of items and their spacing
    float menuHeight = (static_cast<int>(ContextMenuItem::COUNT) * ITEM_HEIGHT) + (static_cast<int>(ContextMenuItem::COUNT) - 1) * (PADDING / 2) + PADDING * 2;

    // Apply boundary checks to keep the menu within screen
    float menuDrawX = m_contextMenuX;
    float menuDrawY = m_contextMenuY;

    if (menuDrawX + MENU_WIDTH + PADDING > width)
        menuDrawX = width - MENU_WIDTH - PADDING;
    if (menuDrawY + menuHeight + PADDING > height)
        menuDrawY = height - menuHeight - PADDING;
    if (menuDrawX < PADDING)
        menuDrawX = PADDING;
    if (menuDrawY < PADDING)
        menuDrawY = PADDING;

    // Menu background
    uiRenderer->drawQuad(menuDrawX, menuDrawY, MENU_WIDTH, menuHeight, glm::vec4(0.2f, 0.2f, 0.2f, 0.8f), orthoProjection); // Dark gray, semi-transparent

    // Menu items
    float currentItemY = menuDrawY + PADDING;

    // Item: Toggle Lighting
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Toggle Lighting (L)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;

    // Item: Toggle Edges
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Toggle Edges (V)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;

    // Item: Toggle Perspective
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Toggle Perspective (P)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;

    // Item: Toggle Background Theme
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Toggle Background (B)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;

    // Item: Toggle Axes Widget
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Toggle Axes (C)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;

    // Item: Auto-center Model
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Auto-center Model (A)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;

    // Item: Import Model
    uiRenderer->drawQuad(menuDrawX + PADDING, currentItemY, MENU_WIDTH - 2 * PADDING, ITEM_HEIGHT, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), orthoProjection);
    textRenderer->renderText("Import Model (I)", menuDrawX + PADDING + 5.0f, currentItemY + ITEM_HEIGHT / 2 - (FONT_SIZE * 16 / 2), FONT_SIZE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection);
    currentItemY += ITEM_HEIGHT + PADDING / 2;
}

void Viewer::onMouseButton(int button, int action, double xpos, double ypos)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        m_showContextMenu = !m_showContextMenu; // Toggle menu visibility
        if (m_showContextMenu)
        {
            m_contextMenuX = static_cast<float>(xpos);
            m_contextMenuY = static_cast<float>(ypos);
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (m_showContextMenu)
        {
            // Check if a menu item was clicked
            float menuHeight = (static_cast<int>(ContextMenuItem::COUNT) * ITEM_HEIGHT) + (static_cast<int>(ContextMenuItem::COUNT) - 1) * (PADDING / 2) + PADDING * 2;

            float menuDrawX = m_contextMenuX;
            float menuDrawY = m_contextMenuY;

            if (menuDrawX + MENU_WIDTH + PADDING > width)
                menuDrawX = width - MENU_WIDTH - PADDING;
            if (menuDrawY + menuHeight + PADDING > height)
                menuDrawY = height - menuHeight - PADDING;
            if (menuDrawX < PADDING)
                menuDrawX = PADDING;
            if (menuDrawY < PADDING)
                menuDrawY = PADDING;

            // Calculate item bounds and check for clicks
            float currentItemY = menuDrawY + PADDING;

            // Function to check if a click is within an item's bounds
            auto isClicked = [&](float itemY)
            {
                float itemXMin = menuDrawX + PADDING;
                float itemXMax = menuDrawX + MENU_WIDTH - PADDING;
                float itemYMin = itemY;
                float itemYMax = itemY + ITEM_HEIGHT;
                return (xpos >= itemXMin && xpos <= itemXMax && ypos >= itemYMin && ypos <= itemYMax);
            };

            // Item: Toggle Lighting
            if (isClicked(currentItemY))
            {
                m_lightingEnabled = !m_lightingEnabled;
                std::cout << "Toggle Lighting: " << (m_lightingEnabled ? "On" : "Off") << std::endl;
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            // Item: Toggle Edges
            if (isClicked(currentItemY))
            {
                switch (m_modelViewMode)
                {
                case ModelViewMode::Shaded:
                    m_modelViewMode = ModelViewMode::Wireframe;
                    std::cout << "Toggle Edges: Wireframe" << std::endl;
                    break;
                case ModelViewMode::Wireframe:
                    m_modelViewMode = ModelViewMode::ShadedWithEdges;
                    std::cout << "Toggle Edges: Shaded with Feature Edges" << std::endl;
                    break;
                case ModelViewMode::ShadedWithEdges:
                    m_modelViewMode = ModelViewMode::Shaded;
                    std::cout << "Toggle Edges: Shaded" << std::endl;
                    break;
                }
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            // Item: Toggle Perspective
            if (isClicked(currentItemY))
            {
                usePerspective = !usePerspective;
                std::cout << "Toggle Perspective: " << (usePerspective ? "Perspective" : "Orthographic") << std::endl;
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            // Item: Toggle Background Theme
            if (isClicked(currentItemY))
            {
                setBackgroundTheme(!m_isDarkTheme);
                std::cout << "Toggle Background: " << (m_isDarkTheme ? "Dark" : "Light") << std::endl;
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            // Item: Toggle Axes Widget
            if (isClicked(currentItemY))
            {
                m_showAxesWidget = !m_showAxesWidget;
                std::cout << "Toggle Axes Widget Visible: " << (m_showAxesWidget ? "true" : "false") << std::endl;
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            // Item: Auto-center Model
            if (isClicked(currentItemY))
            {
                autoCenterAndOrientModel();
                std::cout << "Auto-centered and oriented model." << std::endl;
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            // Item: Import Model
            if (isClicked(currentItemY))
            {
                char const *lTheOpenFileName;
                char const *lFilterPatterns[2] = {"*.obj", "*.stl"};

                lTheOpenFileName = tinyfd_openFileDialog(
                    "Open 3D Model", "", 2, lFilterPatterns, "3D Model Files (*.obj, *.stl)", 0);

                if (lTheOpenFileName)
                {
                    std::string filePath(lTheOpenFileName);
                    loadModel(filePath);
                    std::cout << "Loaded model: " << filePath << std::endl;
                }
                else
                {
                    std::cout << "No file selected." << std::endl;
                }
            }
            currentItemY += ITEM_HEIGHT + PADDING / 2;

            m_showContextMenu = false; // Dismiss menu after selection
        }
        firstMouse = true; // Reset firstMouse to avoid jump when orbiting after menu close
    }
}