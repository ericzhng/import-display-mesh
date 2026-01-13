#include "Camera.h"
#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    Position = position;
    Target = target;
    WorldUp = up;
    MouseSensitivity = SENSITIVITY;
    Zoom = ZOOM;
    
    // Initialize standard vectors
    Radius = glm::distance(Position, Target);
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Target, Up);
}

void Camera::ProcessMouseOrbit(float xoffset, float yoffset)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    // 1. Rotate around WorldUp (Yaw - horizontal mouse)
    // Dragging left (xoffset < 0) -> Negative angle -> Camera orbits CW -> Object rotates CCW (Left)
    float angleYaw = glm::radians(-xoffset); 
    glm::quat rotationYaw = glm::angleAxis(angleYaw, WorldUp);

    // 2. Rotate around Camera Right (Pitch - vertical mouse)
    // Dragging up (yoffset > 0) -> Negative angle -> Camera orbits Down -> Object rotates Up
    float anglePitch = glm::radians(yoffset);
    glm::quat rotationPitch = glm::angleAxis(anglePitch, Right);

    // Combine rotations
    glm::quat rotation = rotationYaw * rotationPitch;

    // Update Position relative to Target
    glm::vec3 direction = Position - Target;
    direction = rotation * direction;
    Position = Target + direction;

    // Update Up vector to allow free tumbling (no gimbal lock)
    Up = rotation * Up;

    // Re-calculate vectors
    updateCameraVectors();
}

void Camera::ProcessMousePanning(float xoffset, float yoffset)
{
    float panSpeed = 0.002f * Radius;
    Target -= Right * xoffset * panSpeed;
    Target -= Up * yoffset * panSpeed;
    
    // Position must move with Target
    // Re-calculate Position based on new Target, keeping relative direction same
    // Actually, just translating both Position and Target is easier
    Position -= Right * xoffset * panSpeed;
    Position -= Up * yoffset * panSpeed;

    // updateCameraVectors(); // Vectors don't change orientation during pan
}

void Camera::ProcessMouseScroll(float yoffset)
{
    float zoomLevel = yoffset * 0.1f * Radius;
    if (Radius - zoomLevel < 0.1f) return;
    
    Radius -= zoomLevel;
    
    // Move Position closer/further from Target
    glm::vec3 direction = glm::normalize(Position - Target);
    Position = Target + direction * Radius;
}

void Camera::SetTarget(glm::vec3 target, float radius)
{
    glm::vec3 direction = glm::normalize(Position - Target);
    Target = target;
    Radius = radius;
    Position = Target + direction * Radius;
    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    // Re-orthogonalize to prevent floating point drift
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, Up)); // Trust Up more than WorldUp for free rotation
    Up = glm::normalize(glm::cross(Right, Front));
}
