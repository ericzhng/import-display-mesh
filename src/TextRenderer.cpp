#include "TextRenderer.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp> // Required for glm::ortho

TextRenderer::TextRenderer(int screenWidth, int screenHeight)
    : Characters(),
      quadVAO(0),
      quadVBO(0),
      textShader(nullptr), // Initialize to nullptr first
      screenWidth(screenWidth),
      screenHeight(screenHeight),
      ft(nullptr),  // Initialize to nullptr
      face(nullptr) // Initialize to nullptr
{
    // Load the new text-specific shaders
    textShader = std::make_unique<Shader>("shaders/text.vs", "shaders/text.fs");

    // Configure VAO/VBO for text rendering (2D quad)
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize FreeType
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // Load a font
    loadFont("C:/Windows/Fonts/calibri.ttf"); // Changed to Segoe UI font
}

TextRenderer::~TextRenderer()
{
    // Clean up FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Delete OpenGL buffers
    if (quadVAO != 0)
    {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }

    // Delete character textures
    for (auto const &[key, val] : Characters)
    {
        glDeleteTextures(1, &val.TextureID);
    }
}

void TextRenderer::loadFont(std::string fontPath)
{
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
        return;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48); // Set width to 0 lets FreeType dynamically calculate the width based on the given height

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph: " << c << std::endl;
            continue;
        }
        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        Characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Restore default byte-alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

CharacterMetrics TextRenderer::getCharacterMetrics(char c, float scale)
{
    CharacterMetrics metrics = {0.0f, 0.0f, 0.0f};
    if (Characters.find(c) != Characters.end())
    {
        Character ch = Characters[c];
        metrics.width = (ch.Advance >> 6) * scale;
        metrics.height = ch.Size.y * scale;
        metrics.yBearing = ch.Bearing.y * scale;
    }
    return metrics;
}

void TextRenderer::renderText(const std::string &text, float x, float y, float scale, const glm::vec4 &color, const glm::mat4 &projection)
{
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textShader->use();
    textShader->setVec4("textColor", color);
    textShader->setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(quadVAO);

    // Iterate through all characters
    for (char c : text)
    {
        if (Characters.find(c) == Characters.end())
        {
            std::cerr << "Character " << c << " not found in map." << std::endl;
            continue;
        }

        Character ch = Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
        // Update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}
