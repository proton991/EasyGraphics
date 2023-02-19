#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
//layout (location = 3) in vec3 aTangent;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec2 vTexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 uProjView;
    mat4 uView;
    mat4 uProjection;
};
layout(std140, binding = 1) uniform Model
{
    mat4 uMvp;
    mat4 uModelView;
    mat4 uNormal;
};

void main()
{
    vViewSpacePosition = vec3(uModelView * vec4(aPos, 1));
    vViewSpaceNormal = normalize(vec3(uNormal * vec4(aNormal, 0)));
    vTexCoords = aTexCoords;
    gl_Position = uMvp * vec4(aPos, 1.0);
}