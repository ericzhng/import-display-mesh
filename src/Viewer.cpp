#include "Viewer.h"
#include "Background.h"
#include "AxesWidget.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Viewer::Viewer(int width, int height)
    : width(width), height(height),
      camera(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      usePerspective(false), firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f),
      m_lightingEnabled(true), m_ambientStrength(0.1f) // Initialize new members
{
    background = std::make_unique<Background>();
    axesWidget = std::make_unique<AxesWidget>();
}

Viewer::~Viewer()
{
}

void Viewer::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Assume shaders are relative to CWD
    mainShader = std::make_unique<Shader>("shaders/shader.vs", "shaders/shader.fs");
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
    if (background)
    {
        background->Draw();
    }

    // 2. Draw Model
    if (model && mainShader)
    {
        mainShader->use();

        // Set lighting uniforms (Headlight)
        mainShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // White light
        mainShader->setVec3("lightPos", camera.GetPosition());          // Light at camera's position
        mainShader->setVec3("viewPos", camera.GetPosition());
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
        mainShader->setMat4("model", glm::mat4(1.0f));

        // Draw Model Solid
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        mainShader->setVec3("objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        model->Draw();
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Draw Model Wireframe Overlay
        if (m_show_edges)
        {
            mainShader->setVec3("objectColor", glm::vec3(0.0f, 0.0f, 0.0f));
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            model->Draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // 3. Draw Axes Widget
        if (axesWidget)
        {
            axesWidget->Draw(view, projection, *mainShader);
        }
    }
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

    if (key == GLFW_KEY_V && action == GLFW_PRESS)
        m_show_edges = !m_show_edges;

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        m_lightingEnabled = !m_lightingEnabled; // Toggle lighting with 'L' key
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