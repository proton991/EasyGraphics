#version 450 core
out vec4 FragColor;
in vec3 vColor;
in vec2 TexCoords;

layout (binding = 3) uniform sampler2D u_baseColorTexture;
uniform vec4 u_baseColorFactor;
void main()
{
//	FragColor = vec4(vColor, 1.0f);
	FragColor = u_baseColorFactor * texture(u_baseColorTexture, TexCoords);
//	FragColor = texture(texture_diffuse1, TexCoords);
}