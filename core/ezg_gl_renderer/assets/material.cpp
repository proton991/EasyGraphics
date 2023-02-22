#include "material.hpp"

namespace ezg::gl {
void PBRMaterial::bind_all_textures(ShaderProgram& shader, PBRSamplerData& data) const {
  shader.set_uniform("uBaseColorFactor", base_color_factor);
  shader.set_uniform("uMetallicFactor", (float)metallic_factor);
  shader.set_uniform("uRoughnessFactor", (float)roughness_factor);
  shader.set_uniform("uEmissiveFactor", emissive_factor);
  shader.set_uniform("uOcclusionStrength", (float)occlusion_strength);
  shader.set_uniform("uAlphaMode", alpha_mode);
  shader.set_uniform("uAlphaCutoff", alpha_cutoff);

  if (textures.contains(PBRComponent::BaseColor)) {
    data.samplers[0] = textures.at(PBRComponent::BaseColor)->get_handle();
    shader.set_uniform("uHasBaseColorMap", true);
  } else {
    shader.set_uniform("uHasBaseColorMap", false);
  }
  if (textures.contains(PBRComponent::MetallicRoughness)) {
    data.samplers[1] = textures.at(PBRComponent::MetallicRoughness)->get_handle();
    shader.set_uniform("uHasMetallicRoughnessMap", true);
  } else {
    shader.set_uniform("uHasMetallicRoughnessMap", false);
  }

  if (textures.contains(PBRComponent::Emissive)) {
    data.samplers[2] = textures.at(PBRComponent::Emissive)->get_handle();
    shader.set_uniform("uHasEmissiveMap", true);
  } else {
    shader.set_uniform("uHasEmissiveMap", false);
  }

  if (textures.contains(PBRComponent::Occlusion)) {
    data.samplers[3] = textures.at(PBRComponent::Occlusion)->get_handle();
    shader.set_uniform("uHasOcclusionMap", true);
  } else {
    shader.set_uniform("uHasOcclusionMap", false);
  }

  if (textures.contains(PBRComponent::Normal)) {
    data.samplers[4] = textures.at(PBRComponent::Normal)->get_handle();
    shader.set_uniform("uHasNormalMap", true);
  } else {
    shader.set_uniform("uHasNormalMap", false);
  }
}
}  // namespace ezg::gl