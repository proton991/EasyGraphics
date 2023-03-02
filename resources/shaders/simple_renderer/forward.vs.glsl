#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec3 vWorldSpacePos;
out vec3 vWorldSpaceNormal;
out vec2 vTexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 uProjView;
    mat4 uView;
    mat4 uProjection;
};
layout(std140, binding = 1) uniform Model
{
    mat4 uModel;
};

void main()
{
    vWorldSpaceNormal = vec3((transpose(inverse(uModel))) * vec4(aNormal, 0.0));
    vWorldSpacePos = vec3(uModel * vec4(aPos, 1.0));
    vTexCoords = aTexCoords;
    gl_Position = uProjView * vec4(vWorldSpacePos, 1.0);
}