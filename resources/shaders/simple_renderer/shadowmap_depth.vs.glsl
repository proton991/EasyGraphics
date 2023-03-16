#version 450 core
layout (location = 0) in vec3 aPos;

uniform mat4 uLightSpaceMat;
uniform mat4 uModelMat;

void main()
{
    gl_Position = uLightSpaceMat * uModelMat * vec4(aPos, 1.0);
}