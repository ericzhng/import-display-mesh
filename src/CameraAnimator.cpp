#include "CameraAnimator.h"
#include <glm/gtc/matrix_transform.hpp> // For glm::mix (linear interpolation)
#include <glm/gtx/common.hpp>           // For glm::smoothstep
#include <iostream>

CameraAnimator::CameraAnimator()
    : m_isAnimating(false),
      m_animationTime(0.0f),
      m_animationDuration(0.0f)
{
}

void CameraAnimator::startAnimation(
    const Camera &currentCamera,
    const glm::vec3 &targetPosition,
    const glm::vec3 &targetTarget,
    const glm::vec3 &targetWorldUp,
    float targetZoom,
    float duration)
{
    m_isAnimating = true;
    m_animationTime = 0.0f;
    m_animationDuration = duration;

    // Capture current camera state as start state
    m_startPosition = currentCamera.GetPosition();
    m_startTarget = currentCamera.GetTarget();
    m_startWorldUp = currentCamera.GetUp();
    m_startZoom = currentCamera.GetZoom();

    // Set target state
    m_endPosition = targetPosition;
    m_endTarget = targetTarget;
    m_endWorldUp = glm::normalize(targetWorldUp); // Ensure target up vector is normalized
    m_endZoom = targetZoom;
}

bool CameraAnimator::updateAnimation(float deltaTime, Camera &camera)
{
    if (!m_isAnimating)
    {
        return false;
    }

    m_animationTime += deltaTime;

    float t = glm::clamp(m_animationTime / m_animationDuration, 0.0f, 1.0f);
    
    // Apply smoothstep for smoother acceleration/deceleration
    t = glm::smoothstep(0.0f, 1.0f, t);

    // Interpolate camera parameters
    glm::vec3 currentPosition = glm::mix(m_startPosition, m_endPosition, t);
    glm::vec3 currentTarget = glm::mix(m_startTarget, m_endTarget, t);
    glm::vec3 currentWorldUp = glm::mix(m_startWorldUp, m_endWorldUp, t);
    currentWorldUp = glm::normalize(currentWorldUp); // Keep 'Up' vector normalized

    float currentZoom = glm::mix(m_startZoom, m_endZoom, t);

    // Apply interpolated values to the camera
    camera.SetPositionAndTarget(currentPosition, currentTarget, currentWorldUp);
    camera.SetZoom(currentZoom);

    if (m_animationTime >= m_animationDuration)
    {
        m_isAnimating = false;
        // Ensure camera is precisely at the end state
        camera.SetPositionAndTarget(m_endPosition, m_endTarget, m_endWorldUp);
        camera.SetZoom(m_endZoom);
        return false; // Animation finished
    }

    return true; // Animation in progress
}

// Private helper (if not using glm::smoothstep)
// float CameraAnimator::smoothstep(float edge0, float edge1, float x) {
//     x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f); 
//     return x * x * (3 - 2 * x);
// }
