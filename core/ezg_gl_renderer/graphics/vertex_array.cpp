#include "vertex_array.hpp"
#include "log.hpp"

namespace ezg::gl {

VertexArray::VertexArray() {
  glCreateVertexArrays(1, &m_id);
}

VertexArray::~VertexArray() {
  glDeleteVertexArrays(1, &m_id);
}

void VertexArray::bind() const {
  glBindVertexArray(m_id);
}

void VertexArray::unbind() const {
  glBindVertexArray(0);
}

void VertexArray::attach_vertex_buffer(const Ref<VertexBuffer>& vertex_buffer) {
  assert(!vertex_buffer->get_buffer_view().get_elements().empty());
  glBindVertexArray(m_id);
  vertex_buffer->bind();
  const auto& view     = vertex_buffer->get_buffer_view();
  const auto& elements = view.get_elements();
  for (int i = 0; i < elements.size(); ++i) {
    glEnableVertexAttribArray(i);
    // only support floats/vecNf for now
    glVertexAttribPointer(i, elements[i].count, GL_FLOAT, GL_FALSE, view.get_stride(),
                          reinterpret_cast<void*>(elements[i].offset));
  }
  m_vertex_buffers.push_back(vertex_buffer);
}

void VertexArray::attach_index_buffer(const Ref<IndexBuffer>& index_buffer) {
  glBindVertexArray(m_id);
  index_buffer->bind();
  m_index_buffer = index_buffer;
}
}  // namespace ezg::gl