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

  Ref<Model> get_model(const std::string& name);

  Ref<Model> load_gltf_model(const std::string& path);

  Ref<Texture2D> load_hdr_texture(const std::string& path);

  Ref<TextureCubeMap> load_cubemap_textures(const std::string& name,
                                            const std::vector<std::string>& face_paths);

  std::string extract_name(const std::string& path);

  void delete_model(const std::string& name);

private:
  ResourceManager() { m_white_texture = Texture2D::CreateDefaultWhite(); };

  std::unordered_map<std::string, Ref<Model>> m_model_cache;
  std::unordered_map<std::string, Ref<Texture2D>> m_hdri_cache;
  std::unordered_map<std::string, Ref<TextureCubeMap>> m_cubemap_cache;
  std::vector<Ref<Texture2D>> m_texture_cache;
  Ref<Texture2D> m_white_texture;

  const std::unordered_map<std::string, int> c_AlphaModeValue = {
      {"OPAQUE", 0},
      {"BLEND", 1},
      {"MASK", 2},
  };
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_RESOURCE_SYSTEM_HPP
