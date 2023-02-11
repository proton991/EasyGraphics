#version 450 core
layout (location = 0) in vec2 in_Pos;
layout (location = 1) in vec2 in_TexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = in_TexCoords;
    gl_Position = vec4(in_Pos.x, in_Pos.y, 0.0, 1.0);
}  