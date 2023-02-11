#ifndef EASYGRAPHICS_VERTEX_ARRAY_HPP
#define EASYGRAPHICS_VERTEX_ARRAY_HPP

#include <glad/glad.h>
#include <memory>
#include <vector>
#include "buffer.hpp"

namespace ezg::gl {
enum BufferType : int { Array = GL_ARRAY_BUFFER, Element = GL_ELEMENT_ARRAY_BUFFER };
enum DrawMode : int { STATIC = GL_STATIC_DRAW, DYNAMIC = GL_DYNAMIC_DRAW, STREAM = GL_STREAM_DRAW };


class VertexArray;
using VertexArrayPtr = std::shared_ptr<VertexArray>;
class VertexArray {
public:
  static VertexArrayPtr Create() {
    return std::make_shared<VertexArray>();
  }
  VertexArray();
  virtual ~VertexArray();

  void bind() const;
  void unbind() const;

  void attach_vertex_buffer(const VertexBufferPtr& vertex_buffer);
  void attach_index_buffer(const IndexBufferPtr& index_buffer);

  const IndexBufferPtr& get_index_buffer() const { return m_index_buffer; }
private:
  uint32_t m_id{0};
  std::vector<VertexBufferPtr> m_vertex_buffers;
  IndexBufferPtr m_index_buffer;
};
}  // namespace ezg::gl

#endif  //EASYGRAPHICS_VERTEX_ARRAY_HPP
