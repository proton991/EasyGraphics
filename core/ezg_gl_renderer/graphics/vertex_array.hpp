#ifndef EASYGRAPHICS_VERTEX_ARRAY_HPP
#define EASYGRAPHICS_VERTEX_ARRAY_HPP

#include <glad/glad.h>
#include <vector>
#include "buffer.hpp"

namespace ezg::gl {
enum BufferType : int { Array = GL_ARRAY_BUFFER, Element = GL_ELEMENT_ARRAY_BUFFER };
enum DrawMode : int { STATIC = GL_STATIC_DRAW, DYNAMIC = GL_DYNAMIC_DRAW, STREAM = GL_STREAM_DRAW };

class VertexArray {
public:
  static Ref<VertexArray> Create() {
    return CreateRef<VertexArray>();
  }
  VertexArray();
  virtual ~VertexArray();

  void bind() const;
  void unbind() const;

  void attach_vertex_buffer(const Ref<VertexBuffer>& vertex_buffer);
  void attach_index_buffer(const Ref<IndexBuffer>& index_buffer);

  const Ref<IndexBuffer>& get_index_buffer() const { return m_index_buffer; }
private:
  uint32_t m_id{0};
  std::vector<Ref<VertexBuffer>> m_vertex_buffers;
  Ref<IndexBuffer> m_index_buffer;
};
}  // namespace ezg::gl

#endif  //EASYGRAPHICS_VERTEX_ARRAY_HPP
