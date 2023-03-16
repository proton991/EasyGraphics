#include "engine.hpp"
#include "log.hpp"
#include "renderer/basic_renderer.hpp"
#include "shadow_scene.hpp"
#include "simple_scene.hpp"
#include "systems/gui_system.hpp"
#include "systems/input_system.hpp"
#include "systems/profile_system.hpp"
#include "systems/window_system.hpp"

using namespace ezg::system;

namespace ezg::gl {
void Engine::initialize(const std::string& active_scene) {
  // setup window
  WindowConfig config{};
  config.width         = 900;
  config.height        = 900;
  config.major_version = 4;
  config.minor_version = 5;
  config.resizable     = GL_TRUE;
  config.title         = "OpenGL Renderer";
  m_window             = CreateRef<Window>(config);

  // setup GUI
  m_gui     = GUISystem::Create(m_window->Handle());
  m_options = CreateRef<RenderOptions>();

  // setup renderer

  RendererConfig render_config{
      config.width,
      config.height,
  };
  m_renderer = CreateRef<BasicRenderer>(render_config);

  // setup scenes
  auto simple_scene = CreateRef<SimpleScene>("SimpleScene");
  simple_scene->init();

  auto shadow_scene = CreateRef<ShadowScene>("ShadowScene");
  shadow_scene->init();

  m_scene_cache.try_emplace(simple_scene->get_name(), std::move(simple_scene));
  m_scene_cache.try_emplace(shadow_scene->get_name(), std::move(shadow_scene));
  spd::info("Activate scene: {}", active_scene);
  m_scene = m_scene_cache.at(active_scene);
  if (!m_scene->has_skybox()) {
    m_options->has_env_map = false;
  }

  // setup camera
  auto aabb = m_scene->get_aabb();
  m_camera  = Camera::Create(aabb.bbx_min, aabb.bbx_max, m_window->get_aspect());

  // timer
  m_stop_watch = CreateRef<StopWatch>();
}

void Engine::load_scene(uint32_t index) {
  m_scene->load_new_model(index);
  m_scene->load_floor();
  m_scene->load_light_model();
  auto aabb = m_scene->get_aabb();
  m_camera  = Camera::Create(aabb.bbx_min, aabb.bbx_max, m_window->get_aspect());
}

void Engine::run() {
  m_options->num_models = m_scene->get_num_models();
  m_options->model_list = m_scene->get_model_data();
  while (!m_window->should_close()) {
    FrameInfo frame_info{m_scene, m_options, m_camera};
    if (m_window->should_resize()) {
      m_window->resize();
      m_renderer->resize_fbos(m_window->get_width(), m_window->get_height());
      float aspect = (float)m_window->get_width() / (float)m_window->get_height();
      m_camera->update_aspect(aspect);
    }
    float delta_time = m_stop_watch->time_step();

    m_scene->update(m_options, delta_time);

    m_camera->update(delta_time, m_options->rotate_camera);

    m_renderer->render_frame(frame_info);

    m_gui->draw(m_options);

    if (m_options->scene_changed) {
      load_scene(m_options->selected_model);
    }
    m_window->swap_buffers();
    m_window->update();
  }
}
}  // namespace ezg::gl