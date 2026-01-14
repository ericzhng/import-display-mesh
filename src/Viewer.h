#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"

class Viewer {
public:
    Viewer(int width, int height);
    ~Viewer();

    void init();
    void loadModel(const std::string& path);
    void render();
    
    // Input Handling
    void onResize(int width, int height);
    void onKey(int key, int action);
    void onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton);
    void onScroll(float yoffset);

    void setPerspective(bool enable) { usePerspective = enable; }
    
private:
    int width, height;
    Camera camera;
    
    std::unique_ptr<Shader> mainShader;
    std::unique_ptr<Shader> bgShader;
    std::unique_ptr<Model> model;
    
    // State
    bool usePerspective;
    float lastX, lastY;
    bool firstMouse;
    
    // Rendering resources
    unsigned int quadVAO, quadVBO;   // Background
    unsigned int axesVAO, axesVBO;   // Axes
    unsigned int labelVAO, labelVBO; // Labels
    
    void initBackground();
    void initAxes();
    void drawBackground();
    void drawAxes(const glm::mat4& view, const glm::mat4& projection);
};
