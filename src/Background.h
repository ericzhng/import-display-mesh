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

private:
    std::unique_ptr<Shader> bgShader;
    unsigned int quadVAO, quadVBO;

    void initBackground();
};

#endif
