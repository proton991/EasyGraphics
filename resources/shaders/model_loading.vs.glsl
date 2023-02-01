#version 450 core
layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_TexCoords;
layout (location = 2) in vec3 in_Normal;
layout (location = 3) in vec3 in_Tangent;


//out vec2 TexCoords;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
//    TexCoords = aTexCoords;
    gl_Position = u_projection * u_view * u_model * vec4(in_Pos, 1.0);
}