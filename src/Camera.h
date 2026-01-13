#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

const float SENSITIVITY = 0.2f;
const float ZOOM = 45.0f;

class Camera
{
public:
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

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f));

    // Returns the view matrix
    glm::mat4 GetViewMatrix();

    // Processes input received from a mouse scroll-wheel event
    void ProcessMouseScroll(float yoffset);

    // Processes input received from a mouse input system (Quaternion based)
    void ProcessMouseOrbit(float xoffset, float yoffset);

    // Processes Panning (Middle Mouse)
    void ProcessMousePanning(float xoffset, float yoffset);

    void SetTarget(glm::vec3 target, float radius);

private:
    void updateCameraVectors();
};

#endif