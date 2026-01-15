#include "UiRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

UiRenderer::UiRenderer()
{
    uiShader = std::make_unique<Shader>("shaders/ui.vs", "shaders/ui.fs");
    setupQuad();
}

UiRenderer::~UiRenderer()
{
    if (quadVAO != 0)
    {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
}

void UiRenderer::setupQuad()
{
    float quadVertices[] = {
        // positions
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void UiRenderer::drawQuad(float x, float y, float width, float height, const glm::vec4& color, const glm::mat4& projection)
{
    glDisable(GL_DEPTH_TEST); // Disable depth test for 2D UI elements
    uiShader->use();
    uiShader->setMat4("projection", projection);
    uiShader->setVec4("uColor", color);

    glBindVertexArray(quadVAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));
    uiShader->setMat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test
}