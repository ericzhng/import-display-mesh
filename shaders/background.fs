#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform bool isDarkTheme; // New uniform to control theme

void main()
{
    // Map [-1, 1] to [0, 1]
    float y = (TexCoords.y + 1.0) * 0.5;

    vec3 bottomColor;
    vec3 topColor;

    if (isDarkTheme) {
        // Blender dark theme colors
        bottomColor = vec3(0.13, 0.13, 0.13); // Dark Gray
        topColor = vec3(0.24, 0.24, 0.24);    // Slightly lighter Dark Gray
    } else {
        // Default light gradient
        bottomColor = vec3(0.8, 0.9, 1.0); // Light Blue
        topColor = vec3(1.0, 1.0, 1.0);    // White
    }

    FragColor = vec4(mix(bottomColor, topColor, y), 1.0);
}