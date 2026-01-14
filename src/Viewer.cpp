#include "Viewer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Viewer::Viewer(int width, int height)
    : width(width), height(height),
      camera(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      usePerspective(false), firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f)
{
}

Viewer::~Viewer()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &labelVAO);
    glDeleteBuffers(1, &labelVBO);
}

void Viewer::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Assume shaders are relative to CWD
    mainShader = std::make_unique<Shader>("shaders/shader.vs", "shaders/shader.fs");
    bgShader = std::make_unique<Shader>("shaders/background.vs", "shaders/background.fs");

    initBackground();
    initAxes();
}

void Viewer::loadModel(const std::string &path)
{
    model = std::make_unique<Model>(path.c_str());
    camera.SetTarget(model->GetCenter(), 10.0f);
}

void Viewer::render()
{
    // Reset Viewport for main scene
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspectRatio = (float)width / (float)height;

    // 1. Draw Background
    glDisable(GL_DEPTH_TEST);
    if (bgShader)
    {
        bgShader->use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glEnable(GL_DEPTH_TEST);

    // 2. Draw Model
    if (model && mainShader)
    {
        mainShader->use();
        glm::mat4 projection;
        if (usePerspective)
            projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 1000.0f);
        else
        {
            float orthoHeight = 2.0f * camera.Radius * tan(glm::radians(camera.Zoom) / 2.0f);
            float orthoWidth = orthoHeight * aspectRatio;
            projection = glm::ortho(-orthoWidth / 2.0f, orthoWidth / 2.0f, -orthoHeight / 2.0f, orthoHeight / 2.0f, 0.1f, 1000.0f);
        }
        glm::mat4 view = camera.GetViewMatrix();
        mainShader->setMat4("projection", projection);
        mainShader->setMat4("view", view);
        mainShader->setMat4("model", glm::mat4(1.0f));

        // Draw Model Solid
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        mainShader->setVec4("objectColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        model->Draw(*mainShader);
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Draw Model Wireframe Overlay
        mainShader->setVec4("objectColor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        model->Draw(*mainShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 3. Draw Axes Widget
        drawAxes(view, projection);
    }
}

void Viewer::drawAxes(const glm::mat4 &view, const glm::mat4 & /*mainProj*/)
{
    // 1. Setup Viewport for the corner widget
    int widgetSize = 120; // Size in pixels
    int margin = 10;      // Distance from bottom-left

    // Enable Scissor Test to restrict clearing to JUST the corner
    glEnable(GL_SCISSOR_TEST);
    glScissor(margin, margin, widgetSize, widgetSize);

    // Set viewport to the corner
    glViewport(margin, margin, widgetSize, widgetSize);

    // Clear depth ONLY in the scissor box so axes draw on top of the main scene
    glClear(GL_DEPTH_BUFFER_BIT);

    // Disable scissor so we don't accidentally clip later operations
    glDisable(GL_SCISSOR_TEST);

    if (!mainShader)
        return;
    mainShader->use();

    // 2. Fixed Matrices (The "No Zoom" Fix)
    // We use a fixed 45 FOV and fixed -3.5f distance.
    // This ignores the Camera.Zoom and Camera.Radius variables.
    glm::mat4 widgetProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);

    // Extract purely the rotation from the main view matrix
    glm::mat4 viewRot = glm::mat4(glm::mat3(view));

    // Move the widget back 3.5 units so it's visible, but rotate it with the camera
    glm::mat4 widgetView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)) * viewRot;

    // Send matrices to shader [cite: 7]
    mainShader->setMat4("projection", widgetProjection);
    mainShader->setMat4("view", widgetView);
    mainShader->setMat4("model", glm::mat4(1.0f));

    // 3. Draw 3D Arrows
    glLineWidth(3.5f);
    glBindVertexArray(axesVAO);

    // X Axis (Red)
    mainShader->setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); //
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_TRIANGLES, 6, 6);

    // Y Axis (Green)
    mainShader->setVec4("objectColor", glm::vec4(0.0f, 0.8f, 0.0f, 1.0f));
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    // Z Axis (Blue)
    mainShader->setVec4("objectColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    glDrawArrays(GL_LINES, 4, 2);
    glDrawArrays(GL_TRIANGLES, 18, 6);

    // 4. Draw 2D Labels
    glDisable(GL_DEPTH_TEST); // Text always on top
    glBindVertexArray(labelVAO);

    // Use Identity matrices for text to draw in pure 2D screen space
    mainShader->setMat4("projection", glm::mat4(1.0f));
    mainShader->setMat4("view", glm::mat4(1.0f));

    // Calculate the Screen Position of the Center (0,0,0)
    glm::vec4 centerClip = widgetProjection * widgetView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec2 centerScreen = glm::vec2(centerClip) / centerClip.w;

    auto drawLabel = [&](glm::vec3 axisDir, glm::vec4 color, int start, int count)
    {
        // A. Project the axis direction to screen space
        glm::vec4 tipClip = widgetProjection * widgetView * glm::vec4(axisDir, 1.0f);
        glm::vec2 tipScreen = glm::vec2(tipClip) / tipClip.w;

        // B. Calculate 2D direction vector from center to tip
        glm::vec2 dir2D = tipScreen - centerScreen;

        // C. Normalize and apply FIXED radius
        //    This prevents the "breathing/rotating" effect.
        //    The label will always be exactly 0.65 NDC units (approx 40px) from the center.
        if (glm::length(dir2D) > 0.001f) // Avoid divide by zero if axis points directly at camera
            dir2D = glm::normalize(dir2D);

        glm::vec2 labelPos = centerScreen + dir2D * 0.8f;

        // D. Construct Model Matrix
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(labelPos, 0.0f));

        // E. Scale: 4.0f makes the 0.1 unit text approx 24 pixels tall in a 120px viewport
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 1.0f));

        mainShader->setMat4("model", model);
        mainShader->setVec4("objectColor", color);
        glDrawArrays(GL_LINES, start, count);
    };

    // Pass pure direction vectors (unit length)
    drawLabel(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0, 4);  // X
    drawLabel(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 0.8f, 0.0f, 1.0f), 4, 6);  // Y
    drawLabel(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 10, 6); // Z

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glLineWidth(0.8f);
}

void Viewer::onResize(int w, int h)
{
    width = w;
    height = h;
}

void Viewer::onKey(int key, int action)
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        usePerspective = !usePerspective;
}

void Viewer::onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton)
{
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
    camera.ProcessMouseScroll(yoffset);
}

void Viewer::initBackground()
{
    float quadVertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f};
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
}

void Viewer::initAxes()
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
}
