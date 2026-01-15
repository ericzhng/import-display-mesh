#include "Window.h"
#include <iostream>
#include <glad/glad.h>

// Constructor
Window::Window(int width, int height, const std::string &title)
    : width(width), height(height), title(title), window(nullptr), eventHandler(nullptr) {}

// Destructor
Window::~Window()
{
    if (window)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

// init()
bool Window::init()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    // Set OpenGL version and profile (good practice)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Uncomment for macOS

    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this); // Use 'this' for callbacks

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback); // Register mouse button callback

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

// other methods
bool Window::shouldClose() const { return glfwWindowShouldClose(window); }
void Window::swapBuffers() { glfwSwapBuffers(window); }
void Window::pollEvents() { glfwPollEvents(); }
void Window::close() { glfwSetWindowShouldClose(window, true); }
void Window::setEventHandler(IEventHandler *handler) { eventHandler = handler; }

// Static Callbacks
void Window::framebuffer_size_callback(GLFWwindow *w, int width, int height)
{
    glViewport(0, 0, width, height);
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(w));
    if (win && win->eventHandler)
        win->eventHandler->onResize(width, height);
}

void Window::key_callback(GLFWwindow *w, int key, int scancode, int action, int mods)
{
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(w));
    if (win && win->eventHandler)
        win->eventHandler->onKey(key, action);
}

void Window::mouse_callback(GLFWwindow *w, double xpos, double ypos)
{
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(w));
    if (win && win->eventHandler)
    {
        bool left = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool middle = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        win->eventHandler->onMouseMove(static_cast<float>(xpos), static_cast<float>(ypos), left, middle);
    }
}

void Window::scroll_callback(GLFWwindow *w, double xoffset, double yoffset)
{
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(w));
    if (win && win->eventHandler)
        win->eventHandler->onScroll(static_cast<float>(yoffset));
}

void Window::mouse_button_callback(GLFWwindow *w, int button, int action, int mods)
{
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(w));
    if (win && win->eventHandler)
    {
        double xpos, ypos;
        glfwGetCursorPos(w, &xpos, &ypos);
        win->eventHandler->onMouseButton(button, action, xpos, ypos);
    }
}