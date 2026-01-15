#include "AxesWidget.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

AxesWidget::AxesWidget() : axesVAO(0), axesVBO(0), labelVAO(0), labelVBO(0)
{
    init();
}

AxesWidget::~AxesWidget()
{
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &labelVAO);
    glDeleteBuffers(1, &labelVBO);
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

    float labelVertices[] = {
        // X
        -0.05f, -0.05f, 0.0f, 0.05f, 0.05f, 0.0f,
        -0.05f, 0.05f, 0.0f, 0.05f, -0.05f, 0.0f,
        // Y
        -0.05f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.05f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, -0.05f, 0.0f,
        // Z
        -0.05f, 0.05f, 0.0f, 0.05f, 0.05f, 0.0f,
        0.05f, 0.05f, 0.0f, -0.05f, -0.05f, 0.0f,
        -0.05f, -0.05f, 0.0f, 0.05f, -0.05f, 0.0f};

    glGenVertexArrays(1, &labelVAO);
    glGenBuffers(1, &labelVBO);
    glBindVertexArray(labelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, labelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(labelVertices), &labelVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

void AxesWidget::Draw(const glm::mat4 &view, const glm::mat4 & /*projection*/, Shader &shader)
{
    // 1. Setup Viewport for the corner widget
    int widgetSize = 120; // Size in pixels
    int margin = 10;      // Distance from bottom-left

    glEnable(GL_SCISSOR_TEST);
    glScissor(margin, margin, widgetSize, widgetSize);
    glViewport(margin, margin, widgetSize, widgetSize);
    glClear(GL_DEPTH_BUFFER_BIT);
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

    // 4. Draw 2D Labels
    // This part is complex and involves projecting the axes tips to screen space
    // to draw the 'X', 'Y', 'Z' labels.
    // For this refactoring, the logic is kept the same.
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(labelVAO);

    shader.setMat4("projection", glm::mat4(1.0f));
    shader.setMat4("view", glm::mat4(1.0f));

    glm::vec4 centerClip = widgetProjection * widgetView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec2 centerScreen = glm::vec2(centerClip) / centerClip.w;

    auto drawLabel = [&](glm::vec3 axisDir, glm::vec4 color, int start, int count)
    {
        glm::vec4 tipClip = widgetProjection * widgetView * glm::vec4(axisDir, 1.0f);
        glm::vec2 tipScreen = glm::vec2(tipClip) / tipClip.w;
        glm::vec2 dir2D = tipScreen - centerScreen;

        if (glm::length(dir2D) > 0.001f)
            dir2D = glm::normalize(dir2D);

        glm::vec2 labelPos = centerScreen + dir2D * 0.8f;
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(labelPos, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 1.0f));

        shader.setMat4("model", model);
        shader.setVec4("objectColor", color);
        glDrawArrays(GL_LINES, start, count);
    };

    drawLabel(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0, 4);  // X
    drawLabel(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.8f, 0.0f, 1.0f), 4, 6);  // Y
    drawLabel(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 10, 6); // Z

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    glBindVertexArray(0);

    shader.setBool("u_isAxesWidget", false); // Reset for subsequent drawing
}
