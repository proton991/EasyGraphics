#ifndef EASYGRAPHICS_BASIC_RENDERER_HPP
#define EASYGRAPHICS_BASIC_RENDERER_HPP

#include <vector>
#include "assets/model.hpp"
#include "graphics/shader.hpp"
#include "graphics/uniform_buffer.hpp"
#include "frame_info.hpp"
namespace ezg::gl {
class BasicRenderer {
public:
  BasicRenderer();
  void init();
  void destroy();

  void render_model(const ModelPtr& model, ShaderProgram& shader_program);
  void render_model(const ModelPtr& model);
  void render_frame(const FrameInfo& info);

private:
  struct ModelData {
    glm::mat4 model_matrix;
  } m_model_data;
  UniformBufferPtr m_model_ubo;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_BASIC_RENDERER_HPP
