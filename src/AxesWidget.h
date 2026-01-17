#ifndef AXESWIDGET_H
#define AXESWIDGET_H

#include "Shader.h"
#include <glm/glm.hpp>
#include <memory> // For std::unique_ptr
#include "TextRenderer.h" // Include TextRenderer
#include "UiRenderer.h"   // Include UiRenderer

enum class Axis {
    X_POS, X_NEG,
    Y_POS, Y_NEG,
    Z_POS, Z_NEG
};

struct AxisLabelHitbox {
    Axis axis;
    glm::vec2 center; // In normalized widget-space coordinates (-1 to 1 for the widget's viewport)
    float radius;     // In normalized widget-space coordinates
};

class IAxesWidgetListener {
public:
    virtual void OnAxesWidgetClick(Axis axis) = 0;
    virtual ~IAxesWidgetListener() = default;
};

class AxesWidget
{
public:
    AxesWidget(int screenWidth, int screenHeight, UiRenderer* uiRenderer, IAxesWidgetListener* listener);
    ~AxesWidget();

    void Draw(const glm::mat4 &view, const glm::mat4 &projection, Shader &shader, int viewportX, int viewportY, int viewportWidth, int viewportHeight);
    bool OnMouseButton(double xpos, double ypos, int button, int action);

private:
    unsigned int axesVAO, axesVBO;
    // Removed labelVAO and labelVBO

    std::unique_ptr<TextRenderer> textRenderer;
    UiRenderer* uiRenderer; // Pointer to the shared UiRenderer
    IAxesWidgetListener* m_listener;

    std::vector<AxisLabelHitbox> m_labelHitboxes;
    int m_widgetViewportX, m_widgetViewportY, m_widgetSize; // Store widget's screen-space position and size

    void init();
};

#endif
