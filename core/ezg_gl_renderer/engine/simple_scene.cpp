#include "managers/resource_manager.hpp"
#include "simple_scene.hpp"
namespace ezg::gl {
void SimpleScene::load_new_model(const std::string& path) {
  m_models.clear();
  auto model = ResourceManager::GetInstance().load_gltf_model(path);
  m_models.push_back(model);
}

}
