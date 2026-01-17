#include "Camera.h"
#include <iostream>
#include <limits> // For std::numeric_limits

// Small epsilon for float comparisons
const float AXIS_ALIGNMENT_EPSILON = 0.7f; // Dot product threshold for considering alignment
const float MIN_DOMINANCE_THRESHOLD = 0.1f; // Minimum dot product to consider an axis dominant, even if not strongly aligned

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    Position = position;
    Target = target;
    WorldUp = up;
    MouseSensitivity = Camera::SENSITIVITY;
    Zoom = Camera::ZOOM;

    // Initialize standard vectors
    Radius = glm::distance(Position, Target);
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Target, Up);
}

void Camera::ProcessMouseOrbit(float xoffset, float yoffset)
{ // 1. Sensitivity
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    // 2. Generate Rotations based on LOCAL axes
    // Note: We use 'Up' (Local), not 'WorldUp'
    float angleYaw = glm::radians(-xoffset);  // Negative for natural "grab and drag" feel
    float anglePitch = glm::radians(yoffset); // Positive for natural up/down

    // Create quaternions
    glm::quat rotationYaw = glm::angleAxis(angleYaw, Up);        // Rotate around local Up
    glm::quat rotationPitch = glm::angleAxis(anglePitch, Right); // Rotate around local Right

    // Combine rotations
    // Order matters slightly, but for small mouse deltas, Yaw * Pitch is standard
    glm::quat rotation = rotationYaw * rotationPitch;

    // 3. Apply Rotation to Position
    glm::vec3 direction = Position - Target;
    direction = rotation * direction;
    Position = Target + direction;

    // 4. IMPORTANT: Apply Rotation to Up Vector
    // This allows the camera to "tumble" and go upside down freely
    Up = rotation * Up;

    // 5. Re-orthogonalize to prevent error accumulation
    // We rely on 'Up' being correct now, so we calculate Right and Front from it
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, Up));
    Up = glm::normalize(glm::cross(Right, Front)); // Enforce 90-degree angles
}

void Camera::ProcessMousePanning(float xoffset, float yoffset, int screenHeight)
{
    // 1. Calculate the height of the view plane at the distance of the pivot (Radius)
    //    Formula: Height = 2 * Distance * tan(FOV / 2)
    float fovRadians = glm::radians(Zoom); // 'Zoom' acts as FOV in your code
    float visibleHeightAtPivot = 2.0f * Radius * tan(fovRadians * 0.5f);

    // 2. Calculate how much world distance corresponds to 1 pixel
    float pixelToWorldRatio = visibleHeightAtPivot / (float)screenHeight;

    // 3. Move logic (Note: vertical yoffset might need inversion depending on mouse callback)
    //    If dragging mouse DOWN (yoffset < 0), we want camera to move UP so object moves DOWN.
    //    Your Viewer.cpp calculates yoffset = lastY - ypos.
    //    If ypos goes up (mouse down), yoffset is negative.
    //    We need Target -= Up * negative. = Target + Up. Camera moves Up. Correct.

    Target -= Right * xoffset * pixelToWorldRatio;
    Target -= Up * yoffset * pixelToWorldRatio;
    Position -= Right * xoffset * pixelToWorldRatio;
    Position -= Up * yoffset * pixelToWorldRatio;
}

void Camera::ProcessMouseScroll(float yoffset)
{
    // Exponential Zoom
    // 0.9f means "get 10% closer" per click. 1.1f means "get 10% further".
    float zoomFactor = 0.9f;

    if (yoffset > 0) // Zoom In
        Radius *= zoomFactor;
    else // Zoom Out
        Radius /= zoomFactor;

    // Update Position
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

void Camera::SetPositionAndTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    Position = position;
    Target = target;
    WorldUp = up; // Store the provided up vector as the world up
    Radius = glm::distance(Position, Target);

    // If camera is placed exactly at the target, move it slightly to avoid issues
    if (Radius < 0.0001f)
    {
        Position += glm::vec3(0.0f, 0.0f, 0.1f); // Move slightly along Z
        Radius = glm::distance(Position, Target);
    }

    // Recalculate camera vectors based on new position, target, and world up
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, WorldUp)); // Use WorldUp to derive Right
    Up = glm::normalize(glm::cross(Right, Front));       // Ensure Up is orthogonal to Front and Right
}

void Camera::updateCameraVectors()
{
    // This existing method will trust the current 'Up' for re-orthogonalization
    // after mouse movements.
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, Up));
    Up = glm::normalize(glm::cross(Right, Front));
}

Axis Camera::GetCurrentViewingAxis() const {
    glm::vec3 normalizedFront = glm::normalize(Front);
    std::cout << "Camera Front: (" << normalizedFront.x << ", " << normalizedFront.y << ", " << normalizedFront.z << ")" << std::endl;

    // Define cardinal axis directions
    std::vector<std::pair<glm::vec3, Axis>> cardinalDirections = {
        {glm::vec3(1.0f, 0.0f, 0.0f), Axis::X_POS},
        {glm::vec3(-1.0f, 0.0f, 0.0f), Axis::X_NEG},
        {glm::vec3(0.0f, 1.0f, 0.0f), Axis::Y_POS},
        {glm::vec3(0.0f, -1.0f, 0.0f), Axis::Y_NEG},
        {glm::vec3(0.0f, 0.0f, 1.0f), Axis::Z_POS},
        {glm::vec3(0.0f, 0.0f, -1.0f), Axis::Z_NEG}
    };

    float maxAbsDotProduct = 0.0f;
    Axis mostDominantAxis = Axis::NONE;
    Axis stronglyAlignedAxis = Axis::NONE;

    std::cout << "Dot Products: ";
    for (const auto& entry : cardinalDirections) {
        float dotProduct = glm::dot(normalizedFront, entry.first);
        std::cout << "Axis " << static_cast<int>(entry.second) << ": " << dotProduct << " ";

        if (dotProduct > AXIS_ALIGNMENT_EPSILON) {
            stronglyAlignedAxis = entry.second; // Found a strongly aligned axis
        }

        if (std::abs(dotProduct) > maxAbsDotProduct) {
            maxAbsDotProduct = std::abs(dotProduct);
            mostDominantAxis = entry.second; // Store the actual axis (POS or NEG)
        }
    }
    std::cout << std::endl;

    // If a strongly aligned axis is found, return it immediately
    if (stronglyAlignedAxis != Axis::NONE) {
        return stronglyAlignedAxis;
    }

    // Otherwise, return the most dominant axis, if it exceeds a minimal threshold
    if (maxAbsDotProduct > MIN_DOMINANCE_THRESHOLD) {
        return mostDominantAxis;
    }

    return Axis::NONE; // Truly not aligned with any dominant primary axis
}