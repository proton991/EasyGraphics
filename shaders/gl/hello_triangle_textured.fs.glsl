#version 450 core
in vec2 TexCoord;
out vec4 FragColor;

// texture sampler
uniform sampler2D texture1;
void main()
{
	FragColor = texture(texture1, TexCoord);
}