#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

class Camera
{
public:
    // static constants
    static constexpr float SENSITIVITY = 0.2f;
    static constexpr float ZOOM = 45.0f;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f));

    // Returns the view matrix
    glm::mat4 GetViewMatrix() const;

    // Getter methods
    glm::vec3 GetPosition() const { return Position; }
    glm::vec3 GetTarget() const { return Target; }
    glm::vec3 GetUp() const { return Up; }
    glm::vec3 GetFront() const { return Front; }
    glm::vec3 GetRight() const { return Right; }
    float GetZoom() const { return Zoom; }
    float GetRadius() const { return Radius; }

    void SetZoom(float zoom) { Zoom = zoom; }

    // Processes input received from a mouse scroll-wheel event
    void ProcessMouseScroll(float yoffset);

    // Processes input received from a mouse input system (Quaternion based)
    void ProcessMouseOrbit(float xoffset, float yoffset);

    // Processes Panning (Middle Mouse)
    void ProcessMousePanning(float xoffset, float yoffset, int screenHeight);

    void SetTarget(glm::vec3 target, float radius);
    void SetPositionAndTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up);

private:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 Target;
    float Radius;

    // Camera options
    float MouseSensitivity;
    float Zoom;

    void updateCameraVectors();
};

#endif