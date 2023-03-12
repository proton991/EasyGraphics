#ifndef CUSTOM_SCENE_HPP
#define CUSTOM_SCENE_HPP

#include "ezg_gl_renderer/engine/scene.hpp"
using namespace ezg::gl;

namespace ezg::app {
class CustomScene : public BaseScene {
public:
  explicit CustomScene(std::string_view name) : BaseScene(name) {}

  void init() override;
};
}

#endif  //CUSTOM_SCENE_HPP
