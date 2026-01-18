#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Camera.h"

class CameraAnimator
{
public:
    CameraAnimator();

    void startAnimation(
        const Camera &currentCamera,
        const glm::vec3 &targetPosition, // This acts as the new "Eye" position
        const glm::vec3 &targetTarget,   // The point the camera looks at
        const glm::vec3 &targetWorldUp,
        float targetZoom,
        float duration);

    bool updateAnimation(float deltaTime, Camera &camera);
    bool isAnimating() const { return m_isAnimating; }

private:
    bool m_isAnimating;
    float m_animationTime;
    float m_animationDuration;

    // We separate the camera state into:
    // 1. The Pivot Point (Target)
    // 2. The Orientation (Quaternion)
    // 3. The Distance from Pivot (Radius)

    // Start state
    glm::vec3 m_startTarget;
    glm::quat m_startOrientation;
    float m_startDistance; // Distance between Pos and Target
    float m_startZoom;     // FOV or Ortho scale

    // End state
    glm::vec3 m_endTarget;
    glm::quat m_endOrientation;
    float m_endDistance;
    float m_endZoom;
};