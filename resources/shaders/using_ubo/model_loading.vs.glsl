#version 450 core
layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_TexCoords;
layout (location = 2) in vec3 in_Normal;
//layout (location = 3) in vec3 in_Tangent;


//out vec2 TexCoords;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_proj_view;
    mat4 u_view;
    mat4 u_projection;
};
out vec3 vColor;
layout(std140, binding = 1) uniform Model
{
    mat4 u_model;
};
//uniform mat4 u_model;
//uniform mat4 u_view;
//uniform mat4 u_projection;

void main()
{
//    TexCoords = aTexCoords;
    vColor = in_Normal;
    gl_Position = u_proj_view * u_model * vec4(in_Pos, 1.0);
}