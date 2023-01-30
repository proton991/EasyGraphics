#include "render_api.hpp"
#include "graphics/vertex_array.hpp"

namespace ezg::gl {

void RenderAPI::set_clear_color(const glm::vec4& color) {
  glClearColor(color.r, color.g, color.b, color.a);
}

void RenderAPI::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderAPI::draw_vertices(const std::shared_ptr<BaseVAO>& vao, uint32_t num_vertices) {
  vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}
}