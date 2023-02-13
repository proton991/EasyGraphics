#ifndef SCENE_HPP
#define SCENE_HPP
#include "assets/model.hpp"
namespace ezg::gl {
class BaseScene;
using BaseScenePtr = std::shared_ptr<BaseScene>;
class BaseScene {
public:
  friend class BasicRenderer;
  explicit BaseScene(std::string_view name);
  // disable copying
  BaseScene& operator=(const BaseScene&) = delete;
  BaseScene(const BaseScene&)            = delete;

  virtual void init() = 0;
  void add_model(const std::string& model_name);

  [[nodiscard]] AABB get_aabb() const;

private:
  std::string m_name;
  std::vector<ModelPtr> m_models;
};
}  // namespace ezg::gl

#endif  //SCENE_HPP
