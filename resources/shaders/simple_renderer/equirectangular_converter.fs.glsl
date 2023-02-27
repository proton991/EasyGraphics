#version 450 core

in vec3 vTexCoords;

uniform sampler2D equirectangularMap;

out vec4 fColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 sampleSphericalMap(const vec3 v){
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    const vec2 uv = sampleSphericalMap(normalize(vTexCoords));
    const vec3 color = texture(equirectangularMap, uv).rgb;

    fColor = vec4(color, 1.0);
}
