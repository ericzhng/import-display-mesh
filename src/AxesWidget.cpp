#include "AxesWidget.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr

AxesWidget::AxesWidget(int screenWidth, int screenHeight, UiRenderer *uiRenderer)
    : axesVAO(0), axesVBO(0), uiRenderer(uiRenderer)
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
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // X
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Y
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Z

        // X Arrowhead
        1.0f, 0.0f, 0.0f, 0.9f, 0.05f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.9f, -0.05f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.05f,

        // Y Arrowhead
        0.0f, 1.0f, 0.0f, 0.05f, 0.9f, 0.0f,
        0.0f, 1.0f, 0.0f, -0.05f, 0.9f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.9f, 0.05f,

        // Z Arrowhead
        0.0f, 0.0f, 1.0f, 0.05f, 0.0f, 0.9f,
        0.0f, 0.0f, 1.0f, -0.05f, 0.0f, 0.9f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.05f, 0.9f};

    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), &axesVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

void AxesWidget::Draw(const glm::mat4 &view, const glm::mat4 & /*projection*/, Shader &shader, int screenWidth, int screenHeight)
{
    // 1. Setup Viewport for the corner widget (Top-Right)
    int widgetSize = 120; // Size in pixels
    int margin = 15;      // Distance from edge (Increased for more offset)

    // Calculate top-right position
    int viewportX = screenWidth - widgetSize - margin;
    int viewportY = screenHeight - widgetSize - margin; // OpenGL's Y is usually bottom-up

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewportX, viewportY, widgetSize, widgetSize);
    glViewport(viewportX, viewportY, widgetSize, widgetSize);
    glClear(GL_DEPTH_BUFFER_BIT); // Clear depth buffer for the widget viewport
    glDisable(GL_SCISSOR_TEST);

    shader.use();
    shader.setBool("u_isAxesWidget", true); // Indicate that we are rendering the axes widget

    // 2. Fixed Matrices for the widget
    glm::mat4 widgetProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 viewRot = glm::mat4(glm::mat3(view)); // Extract rotation
    glm::mat4 widgetView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)) * viewRot;

    shader.setMat4("projection", widgetProjection);
    shader.setMat4("view", widgetView);
    shader.setMat4("model", glm::mat4(1.0f));

    // 3. Draw 3D Arrows
    glLineWidth(3.5f);
    glBindVertexArray(axesVAO);

    shader.setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // X Axis (Red)
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_TRIANGLES, 6, 6);

    shader.setVec4("objectColor", glm::vec4(0.0f, 0.8f, 0.0f, 1.0f)); // Y Axis (Green)
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    shader.setVec4("objectColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Z Axis (Blue)
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
    float labelOffsetFactor = 0.9f;  // How far labels are from the center
    float labelCircleRadius = 15.0f; // Radius of the background circle
    float fontSize = 0.6f;           // Scale for the text labels

    auto drawAxisLabel = [&](glm::vec3 axisDir, const std::string &label, glm::vec4 axisColor)
    {
        glm::vec4 tipClip = widgetProjection * widgetView * glm::vec4(axisDir, 1.0f);
        glm::vec2 tipScreenNDC = glm::vec2(tipClip) / tipClip.w;

        glm::vec2 dir2D = tipScreenNDC - centerScreenNDC;
        if (glm::length(dir2D) > 0.001f)
            dir2D = glm::normalize(dir2D);

        // Position the label circles within the widget's screen space
        glm::vec2 labelCenterPosNDC = centerScreenNDC + dir2D * labelOffsetFactor;

        // Convert labelCenterPosNDC (-1 to 1) to widget space (0 to widgetSize)
        float labelX = (labelCenterPosNDC.x * 0.5f + 0.5f) * widgetSize;
        float labelY = (labelCenterPosNDC.y * 0.5f + 0.5f) * widgetSize;

        // Draw background circle
        if (uiRenderer)
        {
            uiRenderer->drawCircle(labelX, labelY, labelCircleRadius, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), orthoProjection); // Opaque dark grey
        }

        // Draw text label
        if (textRenderer)
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

            // Convert textRenderX and textRenderY from widget-space to screen-space
            // textRenderX += viewportX; // REMOVED: Coordinates should be relative to widget viewport
            // textRenderY += viewportY; // REMOVED: Coordinates should be relative to widget viewport
            textRenderer->renderText(label, textRenderX, textRenderY, fontSize, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), orthoProjection); // White text
        }
    };

    drawAxisLabel(glm::vec3(1.0f, 0.0f, 0.0f), "X", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawAxisLabel(glm::vec3(0.0f, 1.0f, 0.0f), "Y", glm::vec4(0.0f, 0.8f, 0.0f, 1.0f));
    drawAxisLabel(glm::vec3(0.0f, 0.0f, 1.0f), "Z", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    glBindVertexArray(0);

    shader.setBool("u_isAxesWidget", false); // Reset for subsequent drawing
}
