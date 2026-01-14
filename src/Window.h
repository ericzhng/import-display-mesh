#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "IEventHandler.h"

class Window
{
public:
    Window(int width, int height, const std::string &title);
    ~Window();

    bool init();
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void close();

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    GLFWwindow *getHandle() const { return window; }

    // Sets the handler for input and window events.
    void setEventHandler(IEventHandler *handler);

private:
    GLFWwindow *window;
    IEventHandler *eventHandler;
    int width;
    int height;
    std::string title;

    // Static callbacks
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
};
