#version 450 core
out vec4 FragColor;
in vec2 vTexCoords;

layout (binding = 3) uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;
void main()
{
	FragColor = uBaseColorFactor * texture(uBaseColorTexture, vTexCoords);
}