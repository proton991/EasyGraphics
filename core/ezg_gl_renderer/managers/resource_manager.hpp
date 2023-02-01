#ifndef EASYGRAPHICS_RESOURCE_SYSTEM_HPP
#define EASYGRAPHICS_RESOURCE_SYSTEM_HPP

#include <filesystem>
#include <string>
#include <unordered_map>
#include "assets/model.hpp"

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


private:
  ResourceManager() = default;
  void release_all();

  std::unordered_map<std::string, ModelPtr> m_model_cache;

};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_RESOURCE_SYSTEM_HPP
