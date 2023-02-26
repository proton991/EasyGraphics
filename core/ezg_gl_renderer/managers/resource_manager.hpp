#ifndef EASYGRAPHICS_RESOURCE_SYSTEM_HPP
#define EASYGRAPHICS_RESOURCE_SYSTEM_HPP

#include <filesystem>
#include <string>
#include <unordered_map>
#include "assets/model.hpp"
#include "assets/texture.hpp"

namespace ezg::gl {

class ResourceManager {
public:
  static auto& GetInstance() {
    static ResourceManager resource_manager;
    return resource_manager;
  }

  static std::string load_shader_source(const std::filesystem::path& path);

  void load_model(const std::string& name, const std::string& path, bool load_material = false);

  ModelPtr get_model(const std::string& name);

  ModelPtr load_gltf_model(const std::string& name, const std::string& path);

  Texture2DPtr load_hdr_texture(const std::string& path);

  Ref<TextureCubeMap> load_cubemap_textures(const std::string& name, const std::vector<std::string>& face_paths);
private:
  ResourceManager() {
      m_white_texture = Texture2D::CreateDefaultWhite();
  };

  std::unordered_map<std::string, ModelPtr> m_model_cache;
  std::unordered_map<std::string, Texture2DPtr> m_hdri_cache;
  std::unordered_map<std::string, Ref<TextureCubeMap>> m_cubemap_cache;
  std::vector<Texture2DPtr> m_texture_cache;
  Texture2DPtr m_white_texture;

  const std::unordered_map<std::string, int> c_AlphaModeValue = {
      {"OPAQUE", 0},
      {"BLEND", 1},
      {"MASK", 2},
  };
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_RESOURCE_SYSTEM_HPP
