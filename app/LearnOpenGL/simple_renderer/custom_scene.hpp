#ifndef CUSTOM_SCENE_HPP
#define CUSTOM_SCENE_HPP

#include "renderer/scene.hpp"
using namespace ezg::gl;

class CustomScene : public BaseScene {
public:
  explicit CustomScene(std::string_view name) : BaseScene(name) {
    CustomScene::init();
  }
  void init() override {
    add_model("helmet");
  }
};
#endif  //CUSTOM_SCENE_HPP
