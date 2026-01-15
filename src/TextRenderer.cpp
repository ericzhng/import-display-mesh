#include "TextRenderer.h"
#include <glm/gtc/matrix_transform.hpp> // Required for glm::ortho

TextRenderer::TextRenderer(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
    // Initialize shader using the existing UI shaders for simplicity
    // In a real font rendering, a specific text shader would be used
    textShader = std::make_unique<Shader>("shaders/ui.vs", "shaders/ui.fs");
    setupQuad();
}

TextRenderer::~TextRenderer()
{
    if (quadVAO != 0)
    {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
}

void TextRenderer::setupQuad()
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

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
{
    // Disable depth test for text rendering
    glDisable(GL_DEPTH_TEST);

    textShader->use();
    textShader->setVec4("uColor", color);

    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    textShader->setMat4("projection", projection);

    glBindVertexArray(quadVAO);

    // Iterate through all characters
    for (char c : text)
    {
        // For now, each character is a simple quad.
        // In proper FreeType rendering, glyph metrics would be used.
        float charWidth = 10.0f * scale; // Placeholder width
        float charHeight = 15.0f * scale; // Placeholder height

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(charWidth, charHeight, 1.0f));
        textShader->setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += charWidth; // Advance cursor for next character
    }
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST); // Re-enable depth test
}
