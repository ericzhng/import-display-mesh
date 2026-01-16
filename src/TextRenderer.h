#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Shader.h"

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

struct CharacterMetrics {
    float width;
    float height;
    float yBearing; // y-offset from baseline to top of glyph bitmap
};

class TextRenderer
{
public:
    TextRenderer(int screenWidth, int screenHeight);
    ~TextRenderer();

    void renderText(const std::string& text, float x, float y, float scale, const glm::vec4& color);
    CharacterMetrics getCharacterMetrics(char c, float scale);


private:
    std::map<char, Character> Characters;
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;
    std::unique_ptr<Shader> textShader;
    int screenWidth, screenHeight;
    FT_Library ft;
    FT_Face face;

    void loadFont(std::string fontPath);
};
