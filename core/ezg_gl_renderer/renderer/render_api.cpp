#include "render_api.hpp"
#include "assets/mesh.hpp"
#include "assets/model.hpp"
#include "graphics/vertex_array.hpp"

namespace ezg::gl {
void RenderAPI::enable_blending(int sfactor, int dfactor) {
  glEnable(GL_BLEND);
  glBlendFunc(sfactor, dfactor);
}
void RenderAPI::set_clear_color(const glm::vec4& color) {
  glClearColor(color.r, color.g, color.b, color.a);
}

void RenderAPI::clear_color_and_depth() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderAPI::clear_color() {
  glClear(GL_COLOR_BUFFER_BIT);
}

void RenderAPI::enable_depth_testing() {
  glEnable(GL_DEPTH_TEST);
}

void RenderAPI::disable_depth_testing() {
  glDisable(GL_DEPTH_TEST);
}

void RenderAPI::draw_line(const std::shared_ptr<VertexArray>& vao, uint32_t num_vertices) {
  vao->bind();
  glLineWidth(2.0f);
  glDrawArrays(GL_LINES, 0, num_vertices);
}

void RenderAPI::draw_vertices(const std::shared_ptr<VertexArray>& vao,
                              uint32_t num_vertices) {
  vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}
void RenderAPI::draw_indices(const std::shared_ptr<VertexArray>& vao) {
  vao->bind();
  glDrawElements(GL_TRIANGLES, vao->get_index_buffer()->get_count(), GL_UNSIGNED_INT, nullptr);
}


void RenderAPI::draw_meshes(const std::vector<Mesh>& meshes) {
  for (const auto& mesh : meshes) {
    mesh.vao->bind();
    glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, nullptr);
  }
}

void RenderAPI::draw_mesh(const Mesh& mesh) {
  mesh.vao->bind();
  glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, nullptr);
}
}  // namespace ezg::gl