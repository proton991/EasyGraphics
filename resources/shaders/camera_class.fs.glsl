#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

// texture samplers
layout (binding = 0) uniform sampler2D texture1;
layout (binding = 1) uniform sampler2D texture2;
void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = mix(texture(texture1, TexCoords), texture(texture2, TexCoords), 0.5);
}