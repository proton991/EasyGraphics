#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <unordered_map>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include "texture.hpp"
#include <string>
namespace ezg::gl {
enum class PBRComponent {
  BaseColor = 0,
  MetallicRoughness,
  Normal,
  Occlusion,
  Emissive,
};
struct PBRMaterial {
  void bind_all_textures() const;

  std::unordered_map<PBRComponent, TexturePtr> textures;
  double metallicFactor{1};   // default 1
  double roughnessFactor{1};
  glm::vec4 base_color_factor{1, 1, 1, 1};
  glm::vec3 emissive_factor{0, 0, 0};
  double occlusion_strength;

};
}
#endif  //MATERIAL_HPP
