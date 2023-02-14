#include "custom_scene.hpp"
#include "managers/resource_manager.hpp"
namespace ezg::app {
void CustomScene::init() {
  auto model = ResourceManager::GetInstance().get_model("helmet");
  model->translate(glm::vec3{0.0f, 0.0f, 0.0f});
  add_model(model);
}
}
