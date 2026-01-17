#include "AxesWidget.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Required for GLFW_MOUSE_BUTTON_LEFT and GLFW_PRESS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr

AxesWidget::AxesWidget(int screenWidth, int screenHeight, UiRenderer *uiRenderer, IAxesWidgetListener* listener)
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
        // Main Lines
        0.0f, 0.0f, 0.0f, 0.7f, 0.0f, 0.0f, // X
        0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f, // Y
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Z

        // X Arrowhead (tip 0.7f, base 0.6f)
        0.7f, 0.0f, 0.0f, 0.6f, 0.05f, 0.0f,
        0.7f, 0.0f, 0.0f, 0.6f, -0.05f, 0.0f,
        0.7f, 0.0f, 0.0f, 0.6f, 0.0f, 0.05f,

        // Y Arrowhead (tip 0.7f, base 0.6f)
        0.0f, 0.7f, 0.0f, 0.05f, 0.6f, 0.0f,
        0.0f, 0.7f, 0.0f, -0.05f, 0.6f, 0.0f,
        0.0f, 0.7f, 0.0f, 0.0f, 0.6f, 0.05f,

        // Z Arrowhead (tip 0.7f, base 0.6f)
        0.0f, 0.0f, 0.7f, 0.05f, 0.0f, 0.6f,
        0.0f, 0.0f, 0.7f, -0.05f, 0.0f, 0.6f,
        0.0f, 0.0f, 0.7f, 0.0f, 0.05f, 0.6f};

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
    glDrawArrays(GL_TRIANGLES, 6, 6);

    shader.setVec4("objectColor", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f)); // Y Axis (Green-ish)
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    shader.setVec4("objectColor", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f)); // Z Axis (Blue-ish)
    glDrawArrays(GL_LINES, 4, 2);
    glDrawArrays(GL_TRIANGLES, 18, 6);

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

    auto drawAxisLabel = [&](glm::vec3 axisDir, const std::string &label, glm::vec4 axisColor, Axis axisEnum)
    {
        glm::vec4 tipClip = widgetProjection * widgetView * glm::vec4(axisDir, 1.0f);
        glm::vec2 tipScreenNDC = glm::vec2(tipClip) / tipClip.w;

        glm::vec2 dir2D = tipScreenNDC - centerScreenNDC;
        float projectedLengthNDC = glm::length(dir2D);
        if (projectedLengthNDC > 0.001f)
            dir2D = glm::normalize(dir2D);
        else
            dir2D = glm::vec2(0.0f); // Avoid division by zero if tip is at center

        // Convert tipScreenNDC to widget space.
        float baseLabelX_widgetSpace = (tipScreenNDC.x * 0.5f + 0.5f) * widgetSize;
        float baseLabelY_widgetSpace = (tipScreenNDC.y * 0.5f + 0.5f) * widgetSize;

        // Add a small fixed pixel offset outwards from the projected tip.
        float pixelOffset = 4.0f; // Fixed pixel amount for offset (reduced)
        float labelX = baseLabelX_widgetSpace + dir2D.x * pixelOffset;
        float labelY = baseLabelY_widgetSpace + dir2D.y * pixelOffset;

        // Store hitbox information (convert labelX, labelY to normalized widget-space)
        AxisLabelHitbox hitbox;
        hitbox.axis = axisEnum;
        hitbox.center = glm::vec2((labelX / (float)widgetSize) * 2.0f - 1.0f, (labelY / (float)widgetSize) * 2.0f - 1.0f);
        hitbox.radius = (labelCircleRadius / (float)widgetSize) * 2.0f; // Radius in normalized widget-space
        m_labelHitboxes.push_back(hitbox);

        // Draw background circle (always, but with potentially transparent color)
        if (uiRenderer)
        {
            glm::vec4 circleColor = (axisEnum == Axis::X_NEG || axisEnum == Axis::Y_NEG || axisEnum == Axis::Z_NEG) ?
                                    glm::vec4(axisColor.r, axisColor.g, axisColor.b, 0.0f) : // Transparent for negative axes
                                    glm::vec4(axisColor.r, axisColor.g, axisColor.b, 1.0f); // Opaque for positive axes
            uiRenderer->drawCircle(labelX, labelY, labelCircleRadius, circleColor, orthoProjection);
        }

        // Draw text label (only for positive axes, or if a label is provided for some reason)
        if (textRenderer && !label.empty() && (axisEnum == Axis::X_POS || axisEnum == Axis::Y_POS || axisEnum == Axis::Z_POS))
        {
            // Get actual metrics of the single character label
            CharacterMetrics charMetrics = textRenderer->getCharacterMetrics(label[0], fontSize);
            float charWidth = charMetrics.width;
            float charHeight = charMetrics.height;
            float charYBearing = charMetrics.yBearing;

            // Adjust text position to be centered within the circle
            // labelX, labelY are currently center of the circle in widget-space
            float textRenderX = labelX - (charWidth * 0.5f);

            // Calculate baseline for vertical centering:
            // labelY (center of circle) - yBearing (offset from baseline to top) + 0.5 * height (half of total glyph bitmap height)
            float textRenderY = labelY - charYBearing + (charHeight * 0.5f);

            textRenderer->renderText(label, textRenderX, textRenderY, fontSize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), orthoProjection); // Black text
        }
    };

    drawAxisLabel(glm::vec3(1.0f, 0.0f, 0.0f), "X", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f), Axis::X_POS);    // Red-ish
    drawAxisLabel(glm::vec3(-1.0f, 0.0f, 0.0f), "", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f), Axis::X_NEG); // Transparent Red-ish
    drawAxisLabel(glm::vec3(0.0f, 1.0f, 0.0f), "Y", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f), Axis::Y_POS);  // Green-ish
    drawAxisLabel(glm::vec3(0.0f, -1.0f, 0.0f), "", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f), Axis::Y_NEG); // Transparent Green-ish
    drawAxisLabel(glm::vec3(0.0f, 0.0f, 1.0f), "Z", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f), Axis::Z_POS);  // Blue-ish
    drawAxisLabel(glm::vec3(0.0f, 0.0f, -1.0f), "", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f), Axis::Z_NEG); // Transparent Blue-ish

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

            for (const auto& hitbox : m_labelHitboxes)
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
