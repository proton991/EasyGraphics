#version 450 core
out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D uScreenTexture;

void main()
{
    vec3 Color = texture(uScreenTexture, vTexCoords).rgb;
    FragColor = vec4(Color, 1.0);
} 