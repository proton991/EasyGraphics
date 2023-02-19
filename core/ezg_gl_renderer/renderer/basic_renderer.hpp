#ifndef EASYGRAPHICS_BASIC_RENDERER_HPP
#define EASYGRAPHICS_BASIC_RENDERER_HPP

#include <vector>
#include "assets/model.hpp"
#include "frame_info.hpp"
#include "graphics/render_target.hpp"
#include "graphics/shader.hpp"
#include "graphics/uniform_buffer.hpp"
#include "graphics/vertex_array.hpp"
#include "renderer_data.hpp"

namespace ezg::gl {
class BaseScene;
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

  void update(const system::Camera& camera);

  uint32_t m_width{0};
  uint32_t m_height{0};

  ModelData m_model_data{};
  CameraData m_camera_data{};

  UniformBufferPtr m_model_ubo;
  UniformBufferPtr m_camera_ubo;

  RenderTargetPtr m_gbuffer;

  VertexArrayPtr m_quad_vao;
  std::unordered_map<std::string, ShaderProgram> m_shader_cache;

  CoordinateAxisData m_axis_data;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_BASIC_RENDERER_HPP
