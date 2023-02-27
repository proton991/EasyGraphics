#version 450 core
out vec4 fColor;
in vec3 vTexCoords;

uniform samplerCube uEnvSampler;

void main()
{
    vec3 envColor = texture(uEnvSampler, vTexCoords).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

    fColor = vec4(envColor, 1.0);
}
