#include "basic_renderer.hpp"
#include "render_api.hpp"
namespace ezg::gl {
BasicRenderer::BasicRenderer() {
  m_model_ubo = UniformBuffer::Create(sizeof(ModelData), 1);
}

void BasicRenderer::render_model(const ModelPtr& model, ShaderProgram& shader_program) {
  for (const auto& mesh : model->get_meshes()) {
    shader_program.set_uniform("u_model", mesh.model_matrix);
    RenderAPI::draw_mesh(mesh);
  }
}

void BasicRenderer::render_model(const ModelPtr& model) {
  for (const auto& mesh : model->get_meshes()) {
    mesh.material.bind_all_textures();
    m_model_data.model_matrix = mesh.model_matrix;
    m_model_ubo->set_data(&m_model_data, sizeof(ModelData));
    RenderAPI::draw_mesh(mesh);
  }
}

void BasicRenderer::render_frame(const FrameInfo& info) {
  for (const auto& mesh : info.model.get_meshes()) {
    info.shader_program.set_uniform("u_baseColorFactor", mesh.material.base_color_factor);
    mesh.material.bind_all_textures();
    m_model_data.model_matrix = mesh.model_matrix;
    m_model_ubo->set_data(&m_model_data, sizeof(ModelData));
    RenderAPI::draw_mesh(mesh);
  }
}
}