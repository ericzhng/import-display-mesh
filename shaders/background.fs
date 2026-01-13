#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

void main()
{
    // Map [-1, 1] to [0, 1]
    float y = (TexCoords.y + 1.0) * 0.5;
    // Gradient: Light Blue (0.8, 0.9, 1.0) at bottom to White (1.0, 1.0, 1.0) at top
    vec3 bottomColor = vec3(0.8, 0.9, 1.0);
    vec3 topColor = vec3(1.0, 1.0, 1.0);
    FragColor = vec4(mix(bottomColor, topColor, y), 1.0);
}