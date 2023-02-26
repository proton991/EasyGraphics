#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <unordered_map>
#include "graphics/shader.hpp"
#include "texture.hpp"
namespace ezg::gl {
struct PBRSamplerData {
  GLuint64 samplers[5];
};

enum class PBRComponent : decltype(0) {
  BaseColor         = 0,
  MetallicRoughness = 1,
  Emissive          = 2,
  Occlusion         = 3,
  Normal            = 4,
};

struct MaterialFactors {
  glm::vec4 baseColorFactor{1, 1, 1, 1};
  float metallicFactor{1.0};
  float roughnessFactor{1.0};
  glm::vec3 emissiveFactor{0, 0, 0};
  float occlusionStrength{1.0};
};

struct PBRMaterial {
  void upload_textures(Ref<ShaderProgram>& shader, PBRSamplerData& sampler_data) const;

  std::unordered_map<PBRComponent, Ref<Texture2D>> textures;
  double metallic_factor{1.0};  // default 1
  double roughness_factor{1.0};
  glm::vec4 base_color_factor{1, 1, 1, 1};
  glm::vec3 emissive_factor{0, 0, 0};
  double occlusion_strength{1.0};

  // 0: opaque  1: blend  2: mask
  int alpha_mode{0};
  float alpha_cutoff{0.5};
  std::string name;
};
}  // namespace ezg::gl
#endif  //MATERIAL_HPP
