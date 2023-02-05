#version 450 core
out vec4 FragColor;
in vec3 vColor;
//in vec2 TexCoords;

//uniform sampler2D texture_diffuse1;

void main()
{
	FragColor = vec4(vColor, 1.0f);
//	FragColor = texture(texture_diffuse1, TexCoords);
}