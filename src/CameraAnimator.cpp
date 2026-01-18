#include "CameraAnimator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

// Helper to Create a Quaternion from LookAt parameters
// This ensures we capture the exact orientation of the camera
glm::quat getOrientation(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
    glm::vec3 direction = glm::normalize(position - target); // Camera points FROM pos TO target (OpenGL convention usually defines Z as coming out of screen)

    // Check for degenerate cases (pos == target)
    if (glm::length(position - target) < 0.0001f)
    {
        return glm::quat(1, 0, 0, 0); // Identity
    }

    // Use glm::lookAt to build the matrix, then cast to Quat.
    // We invert it because lookAt creates a View Matrix (World->Camera),
    // but we want the Camera's Orientation in World Space (Camera->World).
    glm::mat4 viewMat = glm::lookAt(position, target, up);
    return glm::quat_cast(glm::inverse(viewMat));
}

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

    // --- Capture Start State ---
    glm::vec3 startPos = currentCamera.GetPosition();
    m_startTarget = currentCamera.GetTarget();
    m_startZoom = currentCamera.GetZoom();

    // Calculate distance (Radius)
    m_startDistance = glm::distance(startPos, m_startTarget);

    // Calculate Orientation Quaternion
    m_startOrientation = getOrientation(startPos, m_startTarget, currentCamera.GetUp());

    // --- Capture End State ---
    m_endTarget = targetTarget;
    m_endZoom = targetZoom;

    // Calculate end distance
    m_endDistance = glm::distance(targetPosition, targetTarget);

    // Calculate End Orientation
    // Note: We use the targetWorldUp provided to ensure the flip happens correctly
    m_endOrientation = getOrientation(targetPosition, targetTarget, targetWorldUp);
}

bool CameraAnimator::updateAnimation(float deltaTime, Camera &camera)
{
    if (!m_isAnimating)
    {
        return false;
    }

    m_animationTime += deltaTime;

    // Normalize time (0.0 to 1.0)
    float t = glm::clamp(m_animationTime / m_animationDuration, 0.0f, 1.0f);

    // Apply smoothstep easing (or use smootherstep: t * t * t * (t * (t * 6 - 15) + 10))
    float smoothT = glm::smoothstep(0.0f, 1.0f, t);

    // 1. Interpolate the Target (Pivot point)
    glm::vec3 currentTarget = glm::mix(m_startTarget, m_endTarget, smoothT);

    // 2. Interpolate the Distance (Radius)
    float currentDistance = glm::mix(m_startDistance, m_endDistance, smoothT);

    // 3. Interpolate the Zoom (FOV)
    float currentZoom = glm::mix(m_startZoom, m_endZoom, smoothT);

    // 4. Interpolate Rotation (SLERP)
    // Slerp handles the "shortest path" arc automatically, even for 180 flips.
    glm::quat currentRotation = glm::slerp(m_startOrientation, m_endOrientation, smoothT);

    // 5. Reconstruct the Camera Position and Up Vector

    // Get the "Forward" vector from the rotation (Camera looks down -Z in local space usually,
    // but strictly speaking, in World space, the direction from Target to Pos is Z axis of the camera rotation)
    // We transform the vector (0,0,1) by our orientation quaternion.
    glm::vec3 offsetDirection = currentRotation * glm::vec3(0.0f, 0.0f, 1.0f);

    // Position = Target + (Direction * Radius)
    glm::vec3 currentPosition = currentTarget + (offsetDirection * currentDistance);

    // Calculate the new Up vector from the orientation
    glm::vec3 currentUp = currentRotation * glm::vec3(0.0f, 1.0f, 0.0f);

    // Apply to camera
    camera.SetPositionAndTarget(currentPosition, currentTarget, currentUp);
    camera.SetZoom(currentZoom);

    // Finish check
    if (m_animationTime >= m_animationDuration)
    {
        m_isAnimating = false;
        // Snap to exact end state to fix floating point drift
        camera.SetPositionAndTarget(
            m_endTarget + (m_endOrientation * glm::vec3(0, 0, 1) * m_endDistance),
            m_endTarget,
            m_endOrientation * glm::vec3(0, 1, 0));
        camera.SetZoom(m_endZoom);
        return false;
    }

    return true;
}