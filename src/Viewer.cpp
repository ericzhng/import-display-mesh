#include "Viewer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Viewer::Viewer(int width, int height) 
    : width(width), height(height), 
      camera(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      usePerspective(true), firstMouse(true), lastX(width/2.0f), lastY(height/2.0f)
{
}

Viewer::~Viewer() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &labelVAO);
    glDeleteBuffers(1, &labelVBO);
}

void Viewer::init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Assume shaders are relative to CWD
    mainShader = std::make_unique<Shader>("shaders/shader.vs", "shaders/shader.fs");
    bgShader = std::make_unique<Shader>("shaders/background.vs", "shaders/background.fs");

    initBackground();
    initAxes();
}

void Viewer::loadModel(const std::string& path) {
    model = std::make_unique<Model>(path.c_str());
    camera.SetTarget(model->GetCenter(), 10.0f);
}

void Viewer::render() {
    // Reset Viewport for main scene
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspectRatio = (float)width / (float)height;

    // 1. Draw Background
    glDisable(GL_DEPTH_TEST);
    if (bgShader) {
        bgShader->use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glEnable(GL_DEPTH_TEST);

    // 2. Draw Model
    if (model && mainShader) {
        mainShader->use();
        glm::mat4 projection;
        if (usePerspective)
            projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 1000.0f);
        else {
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

void Viewer::drawAxes(const glm::mat4& view, const glm::mat4& /*mainProj*/) {
    // Clear Depth for widget to draw on top
    glClear(GL_DEPTH_BUFFER_BIT);
    
    int widgetSize = 120;
    glViewport(10, 10, widgetSize, widgetSize);

    if (!mainShader) return;
    mainShader->use();

    // Widget projection and view
    glm::mat4 widgetProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 viewRot = glm::mat4(glm::mat3(view)); // Remove translation
    glm::mat4 widgetView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)) * viewRot;

    mainShader->setMat4("projection", widgetProjection);
    mainShader->setMat4("view", widgetView);
    mainShader->setMat4("model", glm::mat4(1.0f));

    glLineWidth(2.5f);
    glBindVertexArray(axesVAO);

    // Draw 3D Axes Lines
    mainShader->setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red X
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 6, 6); 
    mainShader->setVec4("objectColor", glm::vec4(0.0f, 0.8f, 0.0f, 1.0f)); // Green Y
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 12, 6);
    mainShader->setVec4("objectColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Blue Z
    glDrawArrays(GL_LINES, 4, 2);
    glDrawArrays(GL_LINES, 18, 6);

    // Labels
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(labelVAO);
    mainShader->setMat4("projection", glm::mat4(1.0f)); // NDC
    mainShader->setMat4("view", glm::mat4(1.0f));       // Identity

    auto drawLabel = [&](glm::vec3 worldPos, glm::vec4 color, int start, int count)
    {
        // Project 3D point to NDC
        glm::vec4 clip = widgetProjection * widgetView * glm::vec4(worldPos, 1.0f);
        glm::vec3 ndc = glm::vec3(clip) / clip.w;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), ndc);
        model = glm::scale(model, glm::vec3(0.12f, 0.12f, 1.0f));

        mainShader->setMat4("model", model);
        mainShader->setVec4("objectColor", color);
        glDrawArrays(GL_LINES, start, count);
    };

    drawLabel(glm::vec3(1.2f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0, 4);  // X
    drawLabel(glm::vec3(0.0f, 1.2f, 0.0f), glm::vec4(0.0f, 0.8f, 0.0f, 1.0f), 4, 6);  // Y
    drawLabel(glm::vec3(0.0f, 0.0f, 1.2f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 10, 6); // Z

    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
}

void Viewer::onResize(int w, int h) {
    width = w;
    height = h;
}

void Viewer::onKey(int key, int action) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        usePerspective = !usePerspective;
}

void Viewer::onMouseMove(double xpos, double ypos, bool leftButton, bool middleButton) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (leftButton) {
        camera.ProcessMouseOrbit(xoffset, yoffset);
    }
    if (middleButton) {
        camera.ProcessMousePanning(xoffset, yoffset);
    }
}

void Viewer::onScroll(double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}

void Viewer::initBackground() {
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

void Viewer::initAxes() {
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
