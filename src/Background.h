#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Shader.h"
#include <memory>

class Background
{
public:
    Background();
    ~Background();

    void Draw();
    void setDarkTheme(bool isDarkTheme);

private:
    std::unique_ptr<Shader> bgShader;
    unsigned int quadVAO, quadVBO;
    bool m_isDarkTheme = false; // Add this member

    void initBackground();
};

#endif
