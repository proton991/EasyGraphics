#include "engine.hpp"
#include "managers/resource_manager.hpp"
#include "renderer/basic_renderer.hpp"
#include "simple_scene.hpp"
#include "systems/gui_system.hpp"
#include "systems/input_system.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"

using namespace ezg::system;

namespace ezg::gl {
void Engine::initialize() {
  // setup window
  WindowConfig config{};
  config.width         = 900;
  config.height        = 900;
  config.major_version = 4;
  config.minor_version = 5;
  config.resizable     = GL_FALSE;
  config.title         = "OpenGL Renderer";
  m_window             = CreateRef<Window>(config);

  // setup GUI
  m_gui                   = GUISystem::Create(m_window->Handle());
  m_gui->m_selected_model = m_current_model_index;

  // setup renderer

  RendererConfig render_config{
      config.width,
      config.height,
  };
  m_renderer = CreateRef<BasicRenderer>(render_config);

  // setup scene with default model
  auto model = ResourceManager::GetInstance().load_gltf_model(ModelPaths[m_current_model_index]);
  m_scene    = CreateRef<SimpleScene>("SimpleScene");
  m_scene->add_model(model);

  // setup camera
  auto aabb = m_scene->get_aabb();
  m_camera  = Camera::Create(aabb.bbx_min, aabb.bbx_max);

  // timer
  m_stop_watch = CreateRef<StopWatch>();
}

void Engine::reload_scene(int index) {
  m_current_model_index = index;
  m_scene->load_new_model(ModelPaths[m_current_model_index]);
  auto aabb = m_scene->get_aabb();
  m_camera  = Camera::Create(aabb.bbx_min, aabb.bbx_max);
  KeyboardMouseInput::GetInstance().reset();
}

void Engine::run() {
  GUIInfo gui_info{ModelNames};
  while (!m_window->should_close()) {
    FrameInfo frame_info{m_scene, *m_camera};
    float delta_time = m_stop_watch->time_step();

    if (m_gui->m_rotate_model) {
      m_scene->update(delta_time);
    } else {
      m_scene->update();
    }

    m_camera->update(delta_time);

    m_renderer->render_frame(frame_info);

    m_gui->draw(gui_info);

    if (m_gui->m_selected_model != m_current_model_index) {
      reload_scene(m_gui->m_selected_model);
    }
    m_window->swap_buffers();
    m_window->update();
  }
}
}  // namespace ezg::gl