#ifndef AXESWIDGET_H
#define AXESWIDGET_H

#include "Shader.h"
#include <glm/glm.hpp>
#include <memory> // For std::unique_ptr
#include "TextRenderer.h" // Include TextRenderer
#include "UiRenderer.h"   // Include UiRenderer

class AxesWidget
{
public:
    AxesWidget(int screenWidth, int screenHeight, UiRenderer* uiRenderer);
    ~AxesWidget();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection, Shader &shader, int viewportX, int viewportY, int viewportWidth, int viewportHeight);

private:
    unsigned int axesVAO, axesVBO;
    // Removed labelVAO and labelVBO

    std::unique_ptr<TextRenderer> textRenderer;
    UiRenderer* uiRenderer; // Pointer to the shared UiRenderer

    void init();
};

#endif
