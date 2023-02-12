#include "material.hpp"

namespace ezg::gl {
void PBRMaterial::bind_all_textures() const {
  if (textures.count(PBRComponent::BaseColor)) {
    textures.at(PBRComponent::BaseColor)->bind(3);
  }

  //    for (const auto& [_, texture] : textures) {
  //      texture->bind(3);
  //    }
}
}