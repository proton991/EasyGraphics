#ifndef CUSTOM_SCENE_HPP
#define CUSTOM_SCENE_HPP

#include "renderer/scene.hpp"
using namespace ezg::gl;

namespace ezg::gl {
class SimpleScene : public BaseScene {
public:
  explicit SimpleScene(std::string_view name) : BaseScene(name) {}
  void load_new_model(const std::string& path);
};
}

#endif  //CUSTOM_SCENE_HPP
