#ifndef IEVENT_HANDLER_H
#define IEVENT_HANDLER_H

// This interface defines a contract for classes that want to handle
// window and input events from the `Window` class.
class IEventHandler
{
public:
    virtual ~IEventHandler() = default;

    // Called when the window is resized.
    virtual void onResize(int width, int height) = 0;

    // Called on a key press, release, or repeat.
    virtual void onKey(int key, int action) = 0;

    // Called when the mouse moves.
    virtual void onMouseMove(float xpos, float ypos, bool leftButton, bool middleButton) = 0;

    // Called on a mouse scroll wheel event.
    virtual void onScroll(float yoffset) = 0;

    // Called on a mouse button event (press or release).
    virtual void onMouseButton(int button, int action, double xpos, double ypos) = 0;
};

#endif
