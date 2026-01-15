#version 330 core
layout (location = 0) in vec2 aPos;
out vec2 vs_quad_pos; // Pass local quad position to fragment shader

uniform mat4 projection; // Orthographic projection matrix
uniform mat4 model;      // Model matrix for 2D transformations

void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    vs_quad_pos = aPos; // Pass the local quad position
}