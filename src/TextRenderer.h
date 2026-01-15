#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Shader.h" // Assuming Shader class is available

class TextRenderer
{
public:
    TextRenderer(int screenWidth, int screenHeight);
    ~TextRenderer();

    // Renders text using placeholder quads for each character.
    // This will be replaced with proper font rendering later.
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec4& color);

private:
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;
    std::unique_ptr<Shader> textShader;
    int screenWidth, screenHeight;

    void setupQuad();
};
