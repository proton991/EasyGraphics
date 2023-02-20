#include "material.hpp"

namespace ezg::gl {
void PBRMaterial::bind_all_textures(ShaderProgram& shader) const {
  shader.set_uniform("uBaseColorFactor", base_color_factor);
  shader.set_uniform("uMetallicFactor", (float)metallic_factor);
  shader.set_uniform("uRoughnessFactor", (float)roughness_factor);
  shader.set_uniform("uEmissiveFactor", emissive_factor);
  shader.set_uniform("uOcclusionStrength", (float)occlusion_strength);
  shader.set_uniform("uAlphaMode", alpha_mode);
  shader.set_uniform("uAlphaCutoff", alpha_cutoff);

  if (textures.contains(PBRComponent::BaseColor)) {
    textures.at(PBRComponent::BaseColor)->bind(2);
    shader.set_uniform("uHasBaseColorMap", true);
  } else {
    shader.set_uniform("uHasBaseColorMap", false);
  }
  if (textures.contains(PBRComponent::MetallicRoughness)) {
    textures.at(PBRComponent::MetallicRoughness)->bind(3);
    shader.set_uniform("uHasMetallicRoughnessMap", true);
  } else {
    shader.set_uniform("uHasMetallicRoughnessMap", false);
  }

  if (textures.contains(PBRComponent::Emissive)) {
    textures.at(PBRComponent::Emissive)->bind(4);
    shader.set_uniform("uHasEmissiveMap", true);
  } else {
    shader.set_uniform("uHasEmissiveMap", false);
  }

  if (textures.contains(PBRComponent::Occlusion)) {
    textures.at(PBRComponent::Occlusion)->bind(5);
    shader.set_uniform("uHasOcclusionMap", true);
  } else {
    shader.set_uniform("uHasOcclusionMap", false);
  }

}
}  // namespace ezg::gl