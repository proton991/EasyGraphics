#version 450 core

out vec4 fColor;

in vec3 vTexCoords;

uniform samplerCube skybox;

void main()
{
    fColor = texture(skybox, vTexCoords);
}