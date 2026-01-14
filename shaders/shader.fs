#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos; // Point light
uniform vec3 viewPos;
uniform bool u_lightingEnabled; // New uniform
uniform float u_ambientStrength; // New uniform

void main()
{
    if (!u_lightingEnabled) {
        FragColor = vec4(objectColor, 1.0); // Render unlit if lighting is disabled
        return;
    }

    // Ambient light - soft constant color
    vec3 ambient = u_ambientStrength * vec3(0.3f, 0.3f, 0.3f); // Use uniform ambient strength

    // Diffuse component
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular component (Blinn-Phong)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfVector), 0.0), 32.0); // 32.0 is shininess
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}