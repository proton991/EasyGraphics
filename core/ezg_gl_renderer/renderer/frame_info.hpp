#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP
#include "engine/render_option.hpp"
#include "ezg_gl_renderer/engine/scene.hpp"
#include "systems/camera_system.hpp"

namespace ezg::gl {
class ShaderProgram;
class Model;
class BaseScene;

struct FrameInfo {
  const Ref<BaseScene>& scene;
  const Ref<RenderOptions>& options;
  const Ref<system::Camera>& camera;
};
}  // namespace ezg::gl
#endif  //FRAME_INFO_HPP
