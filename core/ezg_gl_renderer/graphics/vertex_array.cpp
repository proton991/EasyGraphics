#include "vertex_array.hpp"
#include <spdlog/spdlog.h>
namespace ezg::gl {
namespace spd = spdlog;

BaseVAO::BaseVAO() {
  glGenVertexArrays(1, &m_id);
}

void BaseVAO::bind() {
  glBindVertexArray(m_id);
  m_bound = true;
}

void BaseVAO::unbind() {
  glBindVertexArray(0);
  m_bound = false;
}

void BaseVAO::enable_attribute(const GLuint index, const GLint size, const int stride,
                               size_t offset) {
  if (!m_bound) {
    spd::warn("Trying to set attribute on unbound VAO!");
  }
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
}

SimpleVAO::~SimpleVAO() {
  if (m_bound) {
    unbind();
    m_bound = false;
  }
  if (m_allocated) {
    glDeleteBuffers(1, &m_vbo);
    m_allocated = false;
  }
  glDeleteVertexArrays(1, &m_id);
}

void SimpleVAO::attach_buffer(GLenum type, size_t size, const void* data, GLenum usage) {
  glGenBuffers(1, &m_vbo);
  glBindBuffer(type, m_vbo);
  glBufferData(type, size, data, usage);
  m_allocated = true;
}

SimpleIndexVAO::~SimpleIndexVAO() {
  if (m_bound) {
    unbind();
    m_bound = false;
  }
  if (m_allocated) {
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    m_allocated = false;
  }
  glDeleteVertexArrays(1, &m_id);
}

void SimpleIndexVAO::attach_buffer(GLenum type, size_t size, const void* data, GLenum usage) {
  switch (type) {
    case GL_ARRAY_BUFFER:
      glGenBuffers(1, &m_vbo);
      glBindBuffer(type, m_vbo);
      glBufferData(type, size, data, usage);
      m_allocated = true;
      break;
    case GL_ELEMENT_ARRAY_BUFFER:
      glGenBuffers(1, &m_ebo);
      glBindBuffer(type, m_ebo);
      glBufferData(type, size, data, usage);
      m_allocated = true;
      break;
    default:
      spd::error("attaching invalid buffer type");
      break;
  };
}

VertexArrayObject::VertexArrayObject() {
  glCreateVertexArrays(1, &m_id);
}

VertexArrayObject::~VertexArrayObject() {
  glDeleteVertexArrays(1, &m_id);
}

void VertexArrayObject::bind() const {
  glBindVertexArray(m_id);
}

void VertexArrayObject::unbind() const {
  glBindVertexArray(0);
}

void VertexArrayObject::attach_vertex_buffer(const VertexBufferPtr& vertex_buffer) {
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

void VertexArrayObject::attach_index_buffer(const IndexBufferPtr& index_buffer) {
  glBindVertexArray(m_id);
  index_buffer->bind();
  m_index_buffer = index_buffer;
}
}  // namespace ezg::gl