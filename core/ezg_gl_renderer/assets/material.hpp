#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <unordered_map>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include "texture.hpp"
#include <string>
namespace ezg::gl {
enum class PBRComponent : decltype(0) {
  BaseColor = 0,
  MetallicRoughness = 1,
  Emissive = 2,
  Occlusion = 3,
  Normal = 4,
};

struct MaterialFactors {
  glm::vec4 baseColorFactor{1, 1, 1, 1};
  float metallicFactor{1.0};
  float roughnessFactor{1.0};
  glm::vec3 emissiveFactor{0, 0, 0};
  float occlusionStrength{1.0};
};

struct PBRMaterial {
  void get_factor_data(MaterialFactors& factors) const;
  void bind_all_textures() const;

  std::unordered_map<PBRComponent, TexturePtr> textures;
  double metallic_factor{1.0};   // default 1
  double roughness_factor{1.0};
  glm::vec4 base_color_factor{1, 1, 1, 1};
  glm::vec3 emissive_factor{0, 0, 0};
  double occlusion_strength{1.0};

};
}
#endif  //MATERIAL_HPP