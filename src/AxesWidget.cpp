#include "AxesWidget.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Required for GLFW_MOUSE_BUTTON_LEFT and GLFW_PRESS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr
#include <algorithm>            // Required for std::sort

AxesWidget::AxesWidget(int screenWidth, int screenHeight, UiRenderer *uiRenderer, IAxesWidgetListener *listener)
    : axesVAO(0), axesVBO(0), uiRenderer(uiRenderer), m_listener(listener)
{
    textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
    init();
}

AxesWidget::~AxesWidget()
{
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
}

void AxesWidget::init()
{
    float axesVertices[] = {
        // Main Lines (from origin to 0.7f along each axis)
        0.0f, 0.0f, 0.0f, 0.7f, 0.0f, 0.0f, // X-axis (red)
        0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f, // Y-axis (green)
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f  // Z-axis (blue)
    };

    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), &axesVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

void AxesWidget::Draw(const glm::mat4 &view, const glm::mat4 & /*projection*/, Shader &shader, int viewportX, int viewportY, int viewportWidth, int viewportHeight)
{
    // Clear hitboxes from previous frame
    m_labelHitboxes.clear();

    // Save current viewport
    GLint originalViewport[4];
    glGetIntegerv(GL_VIEWPORT, originalViewport);

    // 1. Setup Viewport for the corner widget (Bottom-Left of the *passed viewport*)
    int widgetSize = 120; // Size in pixels
    int margin = 5;       // Distance from edge

    // Calculate bottom-left position within the passed viewport
    // The AxesWidget will draw at (viewportX + margin, viewportY + margin)
    int widgetViewportX = viewportX + margin;
    int widgetViewportY = viewportY + margin; // OpenGL's (0,0) is bottom-left

    // Store these for mouse interaction
    m_widgetViewportX = widgetViewportX;
    m_widgetViewportY = widgetViewportY;
    m_widgetSize = widgetSize;

    glEnable(GL_SCISSOR_TEST);
    glScissor(widgetViewportX, widgetViewportY, widgetSize, widgetSize);
    glViewport(widgetViewportX, widgetViewportY, widgetSize, widgetSize);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear depth buffer for the widget viewport
    glDisable(GL_SCISSOR_TEST);

    shader.use();
    shader.setBool("u_isAxesWidget", true); // Indicate that we are rendering the axes widget

    // 2. Fixed Matrices for the widget
    glm::mat4 widgetProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);          // Increased FoV
    glm::mat4 viewRot = glm::mat4(glm::mat3(view));                                                 // Extract rotation
    glm::mat4 widgetView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)) * viewRot; // Moved camera further back

    shader.setMat4("projection", widgetProjection);
    shader.setMat4("view", widgetView);
    shader.setMat4("model", glm::mat4(1.0f));

    // 3. Draw 3D Arrows
    glLineWidth(2.0f); // Thinner axes lines
    glBindVertexArray(axesVAO);

    shader.setVec4("objectColor", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f)); // X Axis (Red-ish)
    glDrawArrays(GL_LINES, 0, 2);

    shader.setVec4("objectColor", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f)); // Y Axis (Green-ish)
    glDrawArrays(GL_LINES, 2, 2);

    shader.setVec4("objectColor", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f)); // Z Axis (Blue-ish)
    glDrawArrays(GL_LINES, 4, 2);

    // 4. Draw 2D Labels (X, Y, Z in circles)
    glDisable(GL_DEPTH_TEST); // Ensure labels are always on top

    // Orthographic projection for UI elements within the widget's viewport
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)widgetSize, 0.0f, (float)widgetSize, -1.0f, 1.0f);

    glm::vec4 centerClip = widgetProjection * widgetView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec2 centerScreenNDC = glm::vec2(centerClip) / centerClip.w;

    // Convert NDC to widget-space coordinates (0 to widgetSize)
    // NDC (-1 to 1) to widget space (0 to widgetSize)
    float labelCircleRadius = 8.0f; // Smaller radius for the background circle
    float fontSize = 0.3f;          // Larger scale for the text labels

    struct LabelDrawData
    {
        glm::vec3 axisDir;
        std::string labelText;
        glm::vec4 axisColor;
        Axis axisEnum;
        float depth;         // Raw Z-component in camera space (positive is further away)
        glm::vec2 screenPos; // Store computed screen position for drawing and hitbox
    };

    std::vector<LabelDrawData> labelsToDraw;

    // std::cout << "--- AxesWidget Label Depths (before sort) ---" << std::endl;
    // Helper to populate label data
    auto collectLabelData = [&](glm::vec3 dir, const std::string &txt, glm::vec4 color, Axis aEnum)
    {
        glm::vec4 tipCameraSpace = widgetView * glm::vec4(dir, 1.0f);
        // std::cout << "Axis: " << static_cast<int>(aEnum) << ", World Dir: (" << dir.x << ", " << dir.y << ", " << dir.z << ")" << ", CamZ: " << tipCameraSpace.z << std::endl;
        glm::vec4 tipClip = widgetProjection * tipCameraSpace;
        glm::vec2 tipScreenNDC = glm::vec2(tipClip) / tipClip.w;

        glm::vec2 dir2D = tipScreenNDC - centerScreenNDC;
        float projectedLengthNDC = glm::length(dir2D);
        if (projectedLengthNDC > 0.001f)
            dir2D = glm::normalize(dir2D);
        else
            dir2D = glm::vec2(0.0f); // Avoid division by zero if tip is at center

        float baseLabelX_widgetSpace = (tipScreenNDC.x * 0.5f + 0.5f) * widgetSize;
        float baseLabelY_widgetSpace = (tipScreenNDC.y * 0.5f + 0.5f) * widgetSize;

        float pixelOffset = 1.0f;
        glm::vec2 finalScreenPos(baseLabelX_widgetSpace + dir2D.x * pixelOffset, baseLabelY_widgetSpace + dir2D.y * pixelOffset);

        // Store hitbox information
        AxisLabelHitbox hitbox;
        hitbox.axis = aEnum;
        hitbox.center = glm::vec2((finalScreenPos.x / (float)widgetSize) * 2.0f - 1.0f, (finalScreenPos.y / (float)widgetSize) * 2.0f - 1.0f);
        hitbox.radius = (labelCircleRadius / (float)widgetSize) * 2.0f;
        m_labelHitboxes.push_back(hitbox);

        labelsToDraw.push_back({dir, txt, color, aEnum, tipCameraSpace.z, finalScreenPos}); // Use raw tipCameraSpace.z for depth
    };

    // Collect all label data
    collectLabelData(glm::vec3(0.7f, 0.0f, 0.0f), "X", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f), Axis::X_POS);
    collectLabelData(glm::vec3(-0.7f, 0.0f, 0.0f), "", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f), Axis::X_NEG);
    collectLabelData(glm::vec3(0.0f, 0.7f, 0.0f), "Y", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f), Axis::Y_POS);
    collectLabelData(glm::vec3(0.0f, -0.7f, 0.0f), "", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f), Axis::Y_NEG);
    collectLabelData(glm::vec3(0.0f, 0.0f, 0.7f), "Z", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f), Axis::Z_POS);
    collectLabelData(glm::vec3(0.0f, 0.0f, -0.7f), "", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f), Axis::Z_NEG);
    // std::cout << "------------------------------------------" << std::endl;

    // Sort labels by depth (farthest first, so closest are drawn last)
    std::sort(labelsToDraw.begin(), labelsToDraw.end(), [](const LabelDrawData &a, const LabelDrawData &b)
              {
                  return a.depth < b.depth; // Sort ascending, so smallest Z (closest) first.
              });

    // std::cout << "--- AxesWidget Labels (sorted order) ---" << std::endl;
    // for (const auto &data : labelsToDraw)
    // {
    // std::cout << "Axis: " << static_cast<int>(data.axisEnum) << ", Depth: " << data.depth << std::endl;
    // }
    // std::cout << "------------------------------------------" << std::endl;
    // Draw sorted labels
    for (const auto &data : labelsToDraw)
    {
        // Draw background circle
        if (uiRenderer)
        {
            if (data.axisEnum == Axis::X_NEG || data.axisEnum == Axis::Y_NEG || data.axisEnum == Axis::Z_NEG)
            {
                glm::vec4 transparentCircleColor = glm::vec4(data.axisColor.r, data.axisColor.g, data.axisColor.b, 0.4f);
                uiRenderer->drawCircle(data.screenPos.x, data.screenPos.y, labelCircleRadius + 1.0f, transparentCircleColor, orthoProjection);
                uiRenderer->drawCircle(data.screenPos.x, data.screenPos.y, labelCircleRadius, transparentCircleColor, orthoProjection);
            }
            else // Positive axes remain opaque
            {
                glm::vec4 circleColor = glm::vec4(data.axisColor.r, data.axisColor.g, data.axisColor.b, 1.0f);
                uiRenderer->drawCircle(data.screenPos.x, data.screenPos.y, labelCircleRadius, circleColor, orthoProjection);
            }
        }

        // Draw text label
        if (textRenderer && !data.labelText.empty() && (data.axisEnum == Axis::X_POS || data.axisEnum == Axis::Y_POS || data.axisEnum == Axis::Z_POS))
        {
            CharacterMetrics charMetrics = textRenderer->getCharacterMetrics(data.labelText[0], fontSize);
            float charWidth = charMetrics.width;
            float charHeight = charMetrics.height;
            float charYBearing = charMetrics.yBearing;

            float textRenderX = data.screenPos.x - (charWidth * 0.5f);
            float textRenderY = data.screenPos.y - charYBearing + (charHeight * 0.5f);

            textRenderer->renderText(data.labelText, textRenderX, textRenderY, fontSize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), orthoProjection); // Black text
        }
    }

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    glBindVertexArray(0);

    shader.setBool("u_isAxesWidget", false); // Reset for subsequent drawing

    // Restore original viewport
    glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
}

bool AxesWidget::OnMouseButton(double xpos, double ypos, int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Check if the click is within the bounds of the axes widget
        if (xpos >= m_widgetViewportX && xpos <= (m_widgetViewportX + m_widgetSize) &&
            ypos >= m_widgetViewportY && ypos <= (m_widgetViewportY + m_widgetSize))
        {
            // Convert screen coordinates to widget-local coordinates
            // Mouse Y coordinates are usually top-down, OpenGL viewport Y is bottom-up
            float localX = static_cast<float>(xpos) - m_widgetViewportX;
            float localY = static_cast<float>(ypos) - m_widgetViewportY; // This is local Y from bottom of widget, bottom-up

            // Convert local pixel coordinates to normalized widget-space (-1 to 1)
            float normalizedX = (localX / m_widgetSize) * 2.0f - 1.0f;
            float normalizedY = (localY / m_widgetSize) * 2.0f - 1.0f; // Corrected: localY is already bottom-up, no flip needed

            glm::vec2 mouseNormalized = glm::vec2(normalizedX, normalizedY);

            for (const auto &hitbox : m_labelHitboxes)
            {
                float dist = glm::distance(mouseNormalized, hitbox.center);
                if (dist < hitbox.radius)
                {
                    // Click detected on this axis label
                    if (m_listener)
                    {
                        m_listener->OnAxesWidgetClick(hitbox.axis);
                    }
                    return true; // Event handled
                }
            }
        }
    }
    return false; // Event not handled
}
