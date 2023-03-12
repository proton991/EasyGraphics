#include "simple_scene.hpp"
#include <glm/gtx/string_cast.hpp>
#include "managers/resource_manager.hpp"
namespace ezg::gl {
void SimpleScene::load_new_model(const std::string& path) {
  m_models.clear();
  auto model = ResourceManager::GetInstance().load_gltf_model(path);
  m_models.push_back(model);
}

void SimpleScene::load_floor() {
  m_floor = ResourceManager::GetInstance().load_gltf_model(FloorPath);

  const auto aabb         = get_aabb();
  const auto scene_size   = glm::length(aabb.diag);
  const auto floor_size   = glm::length(m_floor->get_aabb().diag);
  const auto scale_factor = 3 * scene_size / floor_size;
  m_floor->scale(scale_factor);
  m_floor->translate(glm::vec3(0.0, aabb.bbx_min.y * 1.01, 0.0));
  ResourceManager::GetInstance().delete_model(m_floor->get_name());
}
}  // namespace ezg::gl
