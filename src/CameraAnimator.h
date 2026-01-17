#pragma once

#include <glm/glm.hpp>
#include "Camera.h" // Assuming Camera.h defines the Camera class

class CameraAnimator
{
public:
    CameraAnimator();

    void startAnimation(
        const Camera &currentCamera,
        const glm::vec3 &targetPosition,
        const glm::vec3 &targetTarget,
        const glm::vec3 &targetWorldUp,
        float targetZoom,
        float duration);

    // Updates the camera state based on delta time and returns true if animation is still in progress
    bool updateAnimation(float deltaTime, Camera &camera);
    bool isAnimating() const { return m_isAnimating; }

private:
    bool m_isAnimating;
    float m_animationTime;
    float m_animationDuration;

    // Start state
    glm::vec3 m_startPosition;
    glm::vec3 m_startTarget;
    glm::vec3 m_startWorldUp;
    float m_startZoom;

    // End state
    glm::vec3 m_endPosition;
    glm::vec3 m_endTarget;
    glm::vec3 m_endWorldUp;
    float m_endZoom;

    // Helper for smoothstep interpolation
    float smoothstep(float edge0, float edge1, float x);
};
