#version 450 core
out vec4 FragColor;

in vec2 vTexCoords;

layout (binding = 0) uniform sampler2D uDepthMap;
uniform float uNear;
uniform float uFar;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
}

void main()
{
    float depthValue = texture(uDepthMap, vTexCoords).r;
//     FragColor = vec4(vec3(LinearizeDepth(depthValue) / uFar), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}