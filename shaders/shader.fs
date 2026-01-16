#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos; // Is now in View Space

uniform vec4 objectColor;
uniform bool u_lightingEnabled;
uniform float u_ambientStrength;
uniform bool u_isAxesWidget;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

void main()
{
    if (u_isAxesWidget) { // If rendering axes, just use objectColor
        FragColor = objectColor;
        return;
    }

    if (!u_lightingEnabled) {
        FragColor = objectColor; // Render unlit if lighting is disabled
        return;
    }

    // Normal is interpolated and in view space from the vertex shader
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(-FragPos); // View direction in view space
    float shininess = 32.0;

    // --- Camera-Space (View-Space) Lighting ---
    // This setup uses specific values to mimic Blender's default studio lighting.

    // Ambient light provides a constant base color
    vec3 ambient = u_ambientStrength * objectColor.rgb; // Use objectColor for ambient light
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    // --- Light 1: Key Light ---
    vec3 light1Dir = normalize(vec3(0.577, 0.577, 0.577));
    vec3 light1DiffColor = vec3(1.0, 1.0, 1.0) * 0.8;
    vec3 light1SpecColor = vec3(1.0, 1.0, 1.0) * 0.8;
    // Diffuse
    float diff1 = max(dot(norm, light1Dir), 0.0);
    totalDiffuse += diff1 * light1DiffColor;
    // Specular
    vec3 half1 = normalize(light1Dir + viewDir);
    float spec1 = pow(max(dot(norm, half1), 0.0), shininess);
    totalSpecular += spec1 * light1SpecColor;

    // --- Light 2: Fill Light ---
    vec3 light2Dir = normalize(vec3(-0.577, 0.0, 0.577));
    vec3 light2DiffColor = vec3(0.5, 0.55, 0.6) * 0.5;
    vec3 light2SpecColor = vec3(0.5, 0.5, 0.5) * 0.5;
    // Diffuse
    float diff2 = max(dot(norm, light2Dir), 0.0);
    totalDiffuse += diff2 * light2DiffColor;
    // Specular
    vec3 half2 = normalize(light2Dir + viewDir);
    float spec2 = pow(max(dot(norm, half2), 0.0), shininess);
    totalSpecular += spec2 * light2SpecColor;

    // --- Light 3: Rim Light ---
    vec3 light3Dir = normalize(vec3(-0.3, 1.0, -1.0));
    vec3 light3DiffColor = vec3(0.7, 0.7, 0.7) * 0.6;
    vec3 light3SpecColor = vec3(0.8, 0.8, 0.8) * 0.6;
    // Diffuse
    float diff3 = max(dot(norm, light3Dir), 0.0);
    totalDiffuse += diff3 * light3DiffColor;
    // Specular
    vec3 half3 = normalize(light3Dir + viewDir);
    float spec3 = pow(max(dot(norm, half3), 0.0), shininess);
    totalSpecular += spec3 * light3SpecColor;
    
    // --- Light 4: Bottom/Bounce Light ---
    vec3 light4Dir = normalize(vec3(0.0, -1.0, 0.0));
    vec3 light4DiffColor = vec3(0.35, 0.35, 0.35) * 0.3;
    vec3 light4SpecColor = vec3(0.0, 0.0, 0.0); // No specular
    // Diffuse
    float diff4 = max(dot(norm, light4Dir), 0.0);
    totalDiffuse += diff4 * light4DiffColor;
    // Specular (contribution is zero)
    vec3 half4 = normalize(light4Dir + viewDir);
    float spec4 = pow(max(dot(norm, half4), 0.0), shininess);
    totalSpecular += spec4 * light4SpecColor;

    // Combine lighting and apply to object color
    vec3 result = (ambient + totalDiffuse) * objectColor.rgb + totalSpecular;

    // Apply tone mapping
    // result = ACESFilm(result);

    FragColor = vec4(result, 1.0);
}