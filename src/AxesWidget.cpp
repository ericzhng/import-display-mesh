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
    // 1. Setup Viewport for the corner widget (Bottom-Left)
    int widgetSize = 120; // Size in pixels (increased to prevent clipping)
    int margin = 5;       // Distance from edge

    // Calculate bottom-left position
    int viewportX = margin;
    int viewportY = margin; // OpenGL's (0,0) is bottom-left

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewportX, viewportY, widgetSize, widgetSize);
    glViewport(viewportX, viewportY, widgetSize, widgetSize);
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
    // float labelOffsetFactor = 0.9f;  // No longer needed
    float labelCircleRadius = 8.0f; // Smaller radius for the background circle
    float fontSize = 0.30f;         // Smaller scale for the text labels

    auto drawAxisLabel = [&](glm::vec3 axisDir, const std::string &label, glm::vec4 axisColor)
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

        // Draw background circle
        if (uiRenderer)
        {
            // Use axis color for circle background, maintain translucency
            glm::vec4 circleColor = glm::vec4(axisColor.r, axisColor.g, axisColor.b, 0.5f);
            uiRenderer->drawCircle(labelX, labelY, labelCircleRadius, circleColor, orthoProjection);
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

            textRenderer->renderText(label, textRenderX, textRenderY, fontSize, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), orthoProjection); // Black text
        }
    };

    drawAxisLabel(glm::vec3(1.0f, 0.0f, 0.0f), "X", glm::vec4(1.0f, 0.2f, 0.32f, 1.0f));  // New Red-ish
    drawAxisLabel(glm::vec3(0.0f, 1.0f, 0.0f), "Y", glm::vec4(0.54f, 0.86f, 0.0f, 1.0f)); // New Green-ish
    drawAxisLabel(glm::vec3(0.0f, 0.0f, 1.0f), "Z", glm::vec4(0.15f, 0.56f, 1.0f, 1.0f)); // New Blue-ish

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    glBindVertexArray(0);

    shader.setBool("u_isAxesWidget", false); // Reset for subsequent drawing
}
