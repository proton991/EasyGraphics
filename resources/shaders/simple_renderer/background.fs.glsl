#version 450 core

out vec4 fColor;

in vec3 vTexCoords;

uniform samplerCube uEnvSampler;

void main()
{
    fColor = texture(uEnvSampler, vTexCoords);
}