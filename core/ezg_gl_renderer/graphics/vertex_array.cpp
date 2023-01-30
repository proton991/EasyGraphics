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
      break ;
    case GL_ELEMENT_ARRAY_BUFFER:
      glGenBuffers(1, &m_ebo);
      glBindBuffer(type, m_ebo);
      glBufferData(type, size, data, usage);
      m_allocated = true;
      break;
    default:
      spd::error("attaching invalid buffer type");
      break ;
  };
}

}  // namespace ezg::gl