#version 330 core
out vec4 FragColor;

in vec2 vs_quad_pos; // Input from vertex shader (local quad position)

uniform vec4 uColor;
uniform bool u_isCircle; // New uniform to indicate if we are drawing a circle

void main()
{
    if (u_isCircle)
    {
        // Calculate distance from the center of the quad (0.5, 0.5)
        float distance_from_center = distance(vs_quad_pos, vec2(0.5, 0.5));
        float radius = 0.5;

        // Simple anti-aliasing for the circle edge
        float alpha = 1.0 - smoothstep(radius - 0.005, radius + 0.005, distance_from_center);
        
        // Discard fragments outside the circle
        if (alpha < 0.01) // Adjust threshold for discarding
        {
            discard;
        }
        FragColor = uColor * alpha; // Apply anti-aliasing to color
    }
    else
    {
        FragColor = uColor;
    }
}