#include "aabb.hpp"
#include "assets/line.hpp"
#include "graphics/buffer.hpp"

namespace ezg::gl {
AABB::AABB(const glm::vec3& _min, const glm::vec3& _max) : bbx_min(_min), bbx_max(_max) {
  diag = bbx_max - bbx_min;
}

void AABB::scale(float factor) {
  auto i  = glm::vec3(1.0f, 0.0f, 0.0f);
  auto j  = glm::vec3(0.0f, 1.0f, 0.0f);
  auto k  = glm::vec3(0.0f, 0.0f, 1.0f);
  float x = glm::dot(diag, i);
  float y = glm::dot(diag, j);
  float z = glm::dot(diag, k);
  x *= factor;
  y *= factor;
  z *= factor;
  bbx_min = glm::vec3(-x * 0.5, -y * 0.5, -z * 0.5);
  bbx_max = glm::vec3(x * 0.5, -y * 0.5, z * 0.5);
  diag    = bbx_max - bbx_min;
}

void AABB::translate(const glm::vec3& pos) {
  glm::vec3 delta = pos - get_center();
  bbx_min += delta;
  bbx_max += delta;
  diag = bbx_max - bbx_min;
}

glm::vec3 AABB::get_center() const {
  return (bbx_min + bbx_max) * 0.5f;
}

void AABB::get_lines_data(Ref<Line>& data) const {
  glm::vec3 i = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 j = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 k = glm::vec3(0.0f, 0.0f, 1.0f);

  float x = glm::dot(diag, i);
  float y = glm::dot(diag, j);
  float z = glm::dot(diag, k);
  auto yellow{glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)};

  auto a = glm::vec3(bbx_min.x, bbx_min.y, bbx_min.z + z);
  auto c = glm::vec3(bbx_min.x + x, bbx_min.y, bbx_min.z);
  auto b = glm::vec3(bbx_min.x + x, bbx_min.y, bbx_min.z + z);
  auto e = glm::vec3(bbx_min.x, bbx_min.y + y, bbx_min.z);
  auto d = glm::vec3(bbx_min.x + x, bbx_min.y + y, bbx_min.z);
  auto f = glm::vec3(bbx_min.x, bbx_min.y + y, bbx_min.z + z);

  data->line_vertices = {
      {bbx_min, yellow}, {a, yellow}, {bbx_min, yellow}, {c, yellow},
      {bbx_min, yellow}, {e, yellow},

      {e, yellow},       {f, yellow}, {e, yellow},       {d, yellow},

      {bbx_max, yellow}, {b, yellow}, {bbx_max, yellow}, {d, yellow},
      {bbx_max, yellow}, {f, yellow},

      {b, yellow},       {a, yellow}, {b, yellow},       {c, yellow},

      {a, yellow},       {f, yellow},

      {d, yellow},       {c, yellow},
  };
  auto vbo = VertexBuffer::Create(data->line_vertices.size() * sizeof(LineVertex),
                                  data->line_vertices.data());
  vbo->set_buffer_view({
      {"aPosition", BufferDataType::Vec3f},
      {"aColor", BufferDataType::Vec4f},
  });
  data->vao = VertexArray::Create();
  data->vao->bind();
  data->vao->attach_vertex_buffer(vbo);
}
}  // namespace ezg::gl