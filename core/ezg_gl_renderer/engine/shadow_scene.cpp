#include "shadow_scene.hpp"
#include <glm/gtx/string_cast.hpp>
#include "assets/skybox.hpp"
#include "managers/resource_manager.hpp"

namespace ezg::gl {
void ShadowScene::init() {
  add_model(ModelPaths[0]);
  load_floor();
  load_light_model();
}

void ShadowScene::load_new_model(uint32_t index) {
  m_models.clear();
  add_model(ModelPaths[index]);
}

void ShadowScene::load_light_model() {
  m_light_model = ResourceManager::GetInstance().load_gltf_model(LightModelPath);

  const auto aabb         = get_aabb();
  const auto scene_size   = glm::length(aabb.diag);
  const auto light_size   = glm::length(m_light_model->get_aabb().diag);
  const auto scale_factor = scene_size / light_size;

  m_light_model->scale(2 * scale_factor);
  m_light_model->translate(aabb.bbx_max * 5.0f);
  ResourceManager::GetInstance().delete_model(m_light_model->get_name());
}

void ShadowScene::load_floor() {
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
