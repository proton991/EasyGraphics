#ifndef EASYGRAPHICS_BASIC_RENDERER_HPP
#define EASYGRAPHICS_BASIC_RENDERER_HPP

#include <vector>
#include "frame_info.hpp"
#include "renderer_data.hpp"

namespace ezg::gl {
// forward declaration
class Framebuffer;
class VertexArray;
class UniformBuffer;
class BaseScene;
class Skybox;

class BasicRenderer {
public:
  struct Config {
    const uint32_t width;
    const uint32_t height;
    const std::vector<ShaderProgramCreateInfo>& shader_program_infos;
  };

  explicit BasicRenderer(const Config& config);

  void render_frame(const FrameInfo& info);

private:
  void compile_shaders(const std::vector<ShaderProgramCreateInfo>& shader_program_infos);
  void setup_ubos();
  void setup_screen_quad();
  void setup_framebuffers(uint32_t width, uint32_t height);
  void setup_coordinate_axis();
  void setup_skybox();

  void update(const system::Camera& camera);

  void set_default_state();

  uint32_t m_width{0};
  uint32_t m_height{0};

  ModelData m_model_data{};
  CameraData m_camera_data{};
  PBRSamplerData m_sampler_data{};

  Ref<UniformBuffer> m_model_ubo;
  Ref<UniformBuffer> m_camera_ubo;
  Ref<UniformBuffer> m_pbr_sampler_ubo;

  Ref<Framebuffer> m_gbuffer;

  Ref<VertexArray> m_quad_vao;
  std::unordered_map<std::string, Ref<ShaderProgram>> m_shader_cache;

  CoordinateAxisData m_axis_data;
  Ref<Skybox> m_skybox;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_BASIC_RENDERER_HPP
