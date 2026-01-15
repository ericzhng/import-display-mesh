#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"

class UiRenderer
{
public:
    UiRenderer();
    ~UiRenderer();

    // Draws a filled rectangle
    void drawQuad(float x, float y, float width, float height, const glm::vec4& color, const glm::mat4& projection);

    // Draws a filled circle
    void drawCircle(float x, float y, float radius, const glm::vec4& color, const glm::mat4& projection);

private:
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;
    std::unique_ptr<Shader> uiShader;

    void setupQuad();
};