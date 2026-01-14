#ifndef AXESWIDGET_H
#define AXESWIDGET_H

#include "Shader.h"
#include <glm/glm.hpp>

class AxesWidget
{
public:
    AxesWidget();
    ~AxesWidget();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection, Shader &shader);

private:
    unsigned int axesVAO, axesVBO;
    unsigned int labelVAO, labelVBO;

    void init();
};

#endif
