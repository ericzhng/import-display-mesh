#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Background.h"
#include "AxesWidget.h"
#include "IEventHandler.h"

class Viewer : public IEventHandler
{
public:
    Viewer(int width, int height);
    ~Viewer();

    void init();
    void loadModel(const std::string &path);
    void render();

    // Input Handling - implementing IEventHandler
    void onResize(int width, int height) override;
    void onKey(int key, int action) override;
    void onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton) override;
    void onScroll(float yoffset) override;

    void setPerspective(bool enable) { usePerspective = enable; }

private:
    int width, height;
    Camera camera;

    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Model> model;
    std::unique_ptr<Background> background;
    std::unique_ptr<AxesWidget> axesWidget;

    // State
    bool usePerspective;
    float lastX, lastY;
    bool firstMouse;
};
