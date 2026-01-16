#include "Background.h"
#include <glad/glad.h>

Background::Background() : quadVAO(0), quadVBO(0)
{
    // Assume shaders are relative to CWD. This path is fragile.
    // A resource manager would be a better solution.
    bgShader = std::make_unique<Shader>("shaders/background.vs", "shaders/background.fs");
    initBackground();
}

Background::~Background()
{
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void Background::Draw()
{
    if (bgShader)
    {
        glDisable(GL_DEPTH_TEST);
        bgShader->use();
        bgShader->setBool("isDarkTheme", m_isDarkTheme); // Pass theme state to shader
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);
    }
}

void Background::setDarkTheme(bool isDarkTheme)
{
    m_isDarkTheme = isDarkTheme;
}

void Background::initBackground()
{
    float quadVertices[] = {
        // positions
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
    glBindVertexArray(0);
}
