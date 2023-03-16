/**
* Reference
* https://www.gsn-lib.org/index.html#projectName=ShadersMonthly09&graphName=MicrofacetBRDF
*/
#version 450 core
#extension GL_ARB_bindless_texture: require
out vec4 fColor;

in vec3 vWorldSpacePos;
in vec4 vLightSpacePos;
in vec3 vWorldSpaceNormal;
in vec2 vTexCoords;

// scene data
uniform vec3 uLightPos;
uniform vec3 uLightDir;
uniform vec3 uLightIntensity;
uniform int uLightType;
uniform vec3 uCameraPos;

// pbr textures
uniform int uAlphaMode;
uniform float uAlphaCutoff;

uniform bool uHasBaseColorMap;
uniform bool uHasMetallicRoughnessMap;
uniform bool uHasEmissiveMap;
uniform bool uHasOcclusionMap;
uniform bool uHasNormalMap;

uniform vec4 uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3 uEmissiveFactor;
uniform float uOcclusionStrength;


// --------- bindless texture ---------------
layout (binding = 2) uniform PBRSamplers {
    sampler2D uPBRSamplers[5];
};

// IBL
layout (binding = 3) uniform samplerCube uEnvDiffuseSampler;
layout (binding = 4) uniform samplerCube uEnvSpecularSampler;
layout (binding = 5) uniform sampler2D uBrdfLutSampler;
layout (binding = 6) uniform sampler2D uShadowMap;

#define TEX_BASECOLOR_INDEX 0
#define TEX_METALLICROUGHNESS_INDEX 1
#define TEX_EMISSIVE_INDEX 2
#define TEX_OCCLUSION_INDEX 3
#define TEX_NORMAL_INDEX 4

#define ALPHAMODE_OPAQUE 0
#define ALPHAMODE_BLEND 1
#define ALPHAMODE_MASK 2

#define SPOT_LIGHT 0
#define DIRECTIONAL_LIGHT 1

#define RECIPROCAL_PI 0.3183098861837907
#define PI 3.1415926535897932384626433832795

const float GAMMA = 2.2;
const float irradiPerp = 1.0;
const float reflectance = 0.5;

const int mipLevelCount = 5;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0) {
        shadow = 0.0;
    }

    return shadow;
}

vec2 directionToSphericalEnvmap(vec3 dir) {
    float phi = atan(dir.y, dir.x);
    float theta = acos(dir.z);
    float s = 0.5 - phi / (2.0 * PI);
    float t = 1.0 - theta / PI;
    return vec2(s, t);
}
vec4 sRGBToLinear(vec4 srgbIn)
{
    return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
}
// from http://www.thetenthplanet.de/archives/1180
mat3 cotangentFrame(in vec3 N, in vec3 p, in vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

vec3 applyNormalMap(in vec3 normal, in vec3 viewVec, in vec2 texcoord)
{
    vec3 highResNormal = texture(uPBRSamplers[TEX_NORMAL_INDEX], texcoord).xyz;
    highResNormal = normalize(highResNormal * 2.0 - 1.0);
    mat3 TBN = cotangentFrame(normal, -viewVec, texcoord);
    return normalize(TBN * highResNormal);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelSchlick90(float cosTheta, float F0, float F90) {
    return F0 + (F90 - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float NoH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float b = (NoH2 * (alpha2 - 1.0) + 1.0);
    return alpha2 * RECIPROCAL_PI / (b * b);
}

float G1_GGX_Schlick(float NoV, float roughness) {
    float alpha = roughness * roughness;
    float k = alpha / 2.0;
    return max(NoV, 0.001) / (NoV * (1.0 - k) + k);
}

float G_Smith(float NoV, float NoL, float roughness) {
    return G1_GGX_Schlick(NoL, roughness) * G1_GGX_Schlick(NoV, roughness);
}

float disneyDiffuseFactor(float NoV, float NoL, float VoH, float roughness) {
    float alpha = roughness * roughness;
    float F90 = 0.5 + 2.0 * alpha * VoH * VoH;
    float F_in = fresnelSchlick90(NoL, 1.0, F90);
    float F_out = fresnelSchlick90(NoV, 1.0, F90);
    return F_in * F_out;
}

vec3 brdfMicrofacet(in vec3 L, in vec3 V, in vec3 N, in float metallic, in float roughness, in vec3 baseColor, in float reflectance) {

    vec3 H = normalize(V + L);

    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);

    vec3 F0 = vec3(0.16 * (reflectance * reflectance));
    F0 = mix(F0, baseColor, metallic);

    vec3 F = fresnelSchlick(VoH, F0);
    float D = D_GGX(NoH, roughness);
    float G = G_Smith(NoV, NoL, roughness);

    vec3 spec = (F * D * G) / (4.0 * max(NoV, 0.001) * max(NoL, 0.001));

    vec3 rhoD = baseColor;

    // optionally
    rhoD *= vec3(1.0) - F;
    rhoD *= disneyDiffuseFactor(NoV, NoL, VoH, roughness);

    rhoD *= (1.0 - metallic);

    vec3 diff = rhoD * RECIPROCAL_PI;

    return diff + spec;
}

// adapted from "Real Shading in Unreal Engine 4", Brian Karis, Epic Games
vec3 specularIBL(vec3 F0, float roughness, vec3 N, vec3 V) {
    float NoV = max(dot(N, V), 0.0);
    vec3 R = reflect(-V, N);
    vec3 T1 = textureLod(uEnvSpecularSampler, R, roughness * float(mipLevelCount)).rgb;
    vec4 brdfIntegration = texture(uBrdfLutSampler, vec2(NoV, roughness));
    vec3 T2 = (F0 * brdfIntegration.x + brdfIntegration.y);
    return T1 * T2;
}

void main() {
    vec3 N = normalize(vWorldSpaceNormal);
    vec3 V = normalize(uCameraPos - vWorldSpacePos);
    if (uHasNormalMap) {
        N = applyNormalMap(N, V, vTexCoords);
    }
    vec3 L;
    if (uLightType == SPOT_LIGHT) {
        L = normalize(uLightPos - vWorldSpacePos); // point light
    } else {
        L = normalize(-uLightDir);  // directional light
    }

    //    vec3 L = normalize(-uLightDir);

    vec3 H = normalize(L + V);

    vec4 baseColor = uBaseColorFactor;
    float roughness = uRoughnessFactor;
    float metallic = uMetallicFactor;
    vec3 radiance = uEmissiveFactor;

    if (uHasBaseColorMap) {
        baseColor *= sRGBToLinear(texture(uPBRSamplers[TEX_BASECOLOR_INDEX], vTexCoords));
    }

    if (uHasMetallicRoughnessMap) {
        // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#pbrmetallicroughnessmetallicroughnesstexture
        // "The metallic-roughness texture.The metalness values are sampled from the B
        // channel.The roughness values are sampled from the G channel."
        vec4 metallicRougnessFromTexture = texture(uPBRSamplers[TEX_METALLICROUGHNESS_INDEX], vTexCoords);
        metallic *= metallicRougnessFromTexture.b;
        roughness *= metallicRougnessFromTexture.g;
    }

    if (uHasEmissiveMap) {
        radiance *= sRGBToLinear(texture(uPBRSamplers[TEX_EMISSIVE_INDEX], vTexCoords)).rgb;
    }


    float irradiance = max(dot(L, N), 0.0) * irradiPerp;
    if (irradiance > 0.0) {
        // if receives light
        vec3 brdf = brdfMicrofacet(L, V, N, metallic, roughness, baseColor.rgb, reflectance);
        // irradiance contribution from directional light
        float distance = length(normalize(uLightPos) - normalize(vWorldSpacePos));
        float attenuation = uLightType == SPOT_LIGHT ? min(1.0 / (distance * distance), 1.0f) : 1.0f;
        radiance += brdf * irradiance * uLightIntensity * attenuation;
        radiance *= (1.0 - ShadowCalculation(vLightSpacePos, N, L));
    }

    // compute F0
    vec3 F0 = vec3(0.16 * (reflectance * reflectance));
    F0 = mix(F0, baseColor.rgb, metallic);
    vec3 F = fresnelSchlick(max(dot(V, H), 0.0), F0);
    vec3 rhoD = (1.0 - metallic) * baseColor.rgb;
    rhoD *= vec3(1.0) - F;

    // IBL Diffuse
    vec3 IBL_Diffuse = rhoD * texture(uEnvDiffuseSampler, N).rgb;
    // IBL Specular
    vec3 IBL_Specular = specularIBL(F0, roughness, N, V);
    if (uHasOcclusionMap) {
        float ao = texture(uPBRSamplers[TEX_OCCLUSION_INDEX], vTexCoords).r;
        radiance = mix(radiance, radiance * ao, uOcclusionStrength);
    }
    vec3 finalColor = IBL_Diffuse + IBL_Specular + radiance;
    if (uAlphaMode == ALPHAMODE_OPAQUE) {
        baseColor.a = 1.0;
    } else if (uAlphaMode == ALPHAMODE_MASK) {
        if (baseColor.a < uAlphaCutoff) {
            discard;
        }
        baseColor.a = 1.0;
    }
    // HDR tonemapping
    finalColor = finalColor / (finalColor + vec3(1.0));

    fColor = vec4(finalColor, baseColor.a);
}