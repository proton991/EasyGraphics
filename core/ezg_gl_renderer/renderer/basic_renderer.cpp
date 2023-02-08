#include "basic_renderer.hpp"
#include "render_api.hpp"
namespace ezg::gl {

void BasicRenderer::render_model(const ModelPtr& model, ShaderProgram& shader_program) {
  for (const auto& mesh : model->get_meshes()) {
    shader_program.set_uniform("u_model", mesh.m_model_matrix);
    RenderAPI::draw_mesh(mesh);
  }
}
}