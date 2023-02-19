#include "material.hpp"

namespace ezg::gl {
void PBRMaterial::get_factor_data(MaterialFactors& factors) const {
  factors.emissiveFactor    = emissive_factor;
  factors.roughnessFactor   = static_cast<float>(roughness_factor);
  factors.metallicFactor    = static_cast<float>(metallic_factor);
  factors.baseColorFactor   = base_color_factor;
  factors.occlusionStrength = static_cast<float>(occlusion_strength);
}


void PBRMaterial::bind_all_textures() const {
  textures.at(PBRComponent::BaseColor)->bind(3);
  if (textures.contains(PBRComponent::MetallicRoughness))
    textures.at(PBRComponent::MetallicRoughness)->bind(4);
  if (textures.contains(PBRComponent::Emissive))
    textures.at(PBRComponent::Emissive)->bind(5);
  if (textures.contains(PBRComponent::Occlusion))
    textures.at(PBRComponent::Occlusion)->bind(6);
}
}  // namespace ezg::gl