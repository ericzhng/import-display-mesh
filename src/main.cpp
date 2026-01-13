#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <iostream>

// Default Camera initialization for Z-up
Camera camera(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
float lastX = 400, lastY = 300;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;
bool usePerspective = true;

// Callbacks
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        usePerspective = !usePerspective;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
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

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        camera.ProcessMouseOrbit(xoffset, yoffset);
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
    {
        camera.ProcessMousePanning(xoffset, yoffset);
    }
}

int main()
{
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "Structural Mesh Viewer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    Shader ourShader("shaders/shader.vs", "shaders/shader.fs");
    Shader backgroundShader("shaders/background.vs", "shaders/background.fs");

    Model myModel("E:/2026-01/OpenGL/import-display-mesh/examples/airboat.obj");
    camera.SetTarget(myModel.GetCenter(), 10.0f);

    // --- Background Quad Setup ---
    float quadVertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f};
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    // --- Axes + Arrows Setup ---
    float axesVertices[] = {
        // Main Lines
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // X-axis (0-1)
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Y-axis (2-3)
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Z-axis (4-5)

        // X Arrowhead (6-11)
        1.0f, 0.0f, 0.0f, 0.9f, 0.05f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.9f, -0.05f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.9f, 0.0f, 0.05f,

        // Y Arrowhead (12-17)
        0.0f, 1.0f, 0.0f, 0.05f, 0.9f, 0.0f,
        0.0f, 1.0f, 0.0f, -0.05f, 0.9f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.9f, 0.05f,

        // Z Arrowhead (18-23)
        0.0f, 0.0f, 1.0f, 0.05f, 0.0f, 0.9f,
        0.0f, 0.0f, 1.0f, -0.05f, 0.0f, 0.9f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.05f, 0.9f};
    unsigned int axesVAO, axesVBO;
    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), &axesVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // --- Labels Setup (Centered at origin) ---
    float labelVertices[] = {
        // X Shape
        -0.05f, -0.05f, 0.0f, 0.05f, 0.05f, 0.0f,
        -0.05f, 0.05f, 0.0f, 0.05f, -0.05f, 0.0f,

        // Y Shape
        -0.05f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.05f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, -0.05f, 0.0f,

        // Z Shape
        -0.05f, 0.05f, 0.0f, 0.05f, 0.05f, 0.0f,
        0.05f, 0.05f, 0.0f, -0.05f, -0.05f, 0.0f,
        -0.05f, -0.05f, 0.0f, 0.05f, -0.05f, 0.0f};
    unsigned int labelVAO, labelVBO;
    glGenVertexArrays(1, &labelVAO);
    glGenBuffers(1, &labelVBO);
    glBindVertexArray(labelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, labelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(labelVertices), &labelVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        float aspectRatio = (float)scrWidth / (float)scrHeight;

        glViewport(0, 0, scrWidth, scrHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Draw Background
        glDisable(GL_DEPTH_TEST);
        backgroundShader.use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        // --- Main Scene ---
        ourShader.use();
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
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", glm::mat4(1.0f));

        // Draw Model
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        ourShader.setVec4("objectColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        myModel.Draw(ourShader);
        glDisable(GL_POLYGON_OFFSET_FILL);

        ourShader.setVec4("objectColor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        myModel.Draw(ourShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // --- Corner Axes Widget ---
        glClear(GL_DEPTH_BUFFER_BIT);
        int widgetSize = 120;
        glViewport(10, 10, widgetSize, widgetSize);

        ourShader.use();
        glm::mat4 widgetProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 viewRot = glm::mat4(glm::mat3(view));
        glm::mat4 widgetView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)) * viewRot;

        ourShader.setMat4("projection", widgetProjection);
        ourShader.setMat4("view", widgetView);
        ourShader.setMat4("model", glm::mat4(1.0f));

        glLineWidth(2.5f);
        glBindVertexArray(axesVAO);

        // Draw 3D Axes
        ourShader.setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        glDrawArrays(GL_LINES, 0, 2);
        glDrawArrays(GL_LINES, 6, 6); // X
        ourShader.setVec4("objectColor", glm::vec4(0.0f, 0.8f, 0.0f, 1.0f));
        glDrawArrays(GL_LINES, 2, 2);
        glDrawArrays(GL_LINES, 12, 6); // Y
        ourShader.setVec4("objectColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        glDrawArrays(GL_LINES, 4, 2);
        glDrawArrays(GL_LINES, 18, 6); // Z

        // --- Flat Billboard Labels ---
        // We draw labels in NDC space so they are perfectly flat and upright
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(labelVAO);
        ourShader.setMat4("projection", glm::mat4(1.0f)); // NDC
        ourShader.setMat4("view", glm::mat4(1.0f));       // Identity

        auto drawLabel = [&](glm::vec3 worldPos, glm::vec4 color, int start, int count)
        {
            // Project 3D point to NDC
            glm::vec4 clip = widgetProjection * widgetView * glm::vec4(worldPos, 1.0f);
            glm::vec3 ndc = glm::vec3(clip) / clip.w;

            // Set model matrix to translate to NDC position
            // We scale the label to stay a constant size on screen
            glm::mat4 model = glm::translate(glm::mat4(1.0f), ndc);
            // Compensate for viewport aspect if needed, but widget is square 1:1
            model = glm::scale(model, glm::vec3(0.12f, 0.12f, 1.0f));

            ourShader.setMat4("model", model);
            ourShader.setVec4("objectColor", color);
            glDrawArrays(GL_LINES, start, count);
        };

        drawLabel(glm::vec3(1.2f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0, 4);  // X
        drawLabel(glm::vec3(0.0f, 1.2f, 0.0f), glm::vec4(0.0f, 0.8f, 0.0f, 1.0f), 4, 6);  // Y
        drawLabel(glm::vec3(0.0f, 0.0f, 1.2f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 10, 6); // Z

        glEnable(GL_DEPTH_TEST);
        glLineWidth(1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &labelVAO);
    glDeleteBuffers(1, &labelVBO);

    glfwTerminate();
    return 0;
}