#ifndef EASYGRAPHICS_VERTEX_ARRAY_HPP
#define EASYGRAPHICS_VERTEX_ARRAY_HPP

#include <glad/glad.h>
#include <vector>

namespace ezg::gl {
enum BufferType : int { Array = GL_ARRAY_BUFFER, Element = GL_ELEMENT_ARRAY_BUFFER };
enum DrawMode : int { STATIC = GL_STATIC_DRAW, DYNAMIC = GL_DYNAMIC_DRAW, STREAM = GL_STREAM_DRAW };

struct ArrayBufferInfo {
  const std::vector<GLfloat>& vertices;
  DrawMode draw_mode{GL_STATIC_DRAW};
};

struct ElementBufferInfo {
  const std::vector<GLfloat>& vertices;
  const std::vector<GLfloat>& indices;
  DrawMode draw_mode{GL_STATIC_DRAW};
};

class BaseVAO {
public:
  virtual ~BaseVAO() = default;
  void bind();
  void unbind();
  void enable_attribute(GLuint index, GLint size, int stride, size_t offset);
  virtual void attach_buffer(GLenum type, size_t size, const void* data, GLenum usage) = 0;
//  virtual void draw() const = 0;

protected:
  BaseVAO();
  GLuint m_id{0};
  int m_num_indices{0};
  GLenum m_draw_mode{GL_TRIANGLES};
  bool m_bound{false};
  bool m_allocated{false};
};

class SimpleVAO : public BaseVAO {
public:
  SimpleVAO() = default;
  ~SimpleVAO() override;
//  void draw() const override;
  void attach_buffer(GLenum type, size_t size, const void* data, GLenum usage) override;

private:
  GLuint m_vbo{0};
};

class SimpleIndexVAO : public BaseVAO {
public:
  SimpleIndexVAO() = default;
  ~SimpleIndexVAO() override;
//  void draw() const override;
  void attach_buffer(GLenum type, size_t size, const void* data, GLenum usage) override;

private:
  GLuint m_vbo{0};
  GLuint m_ebo{0};
};
}  // namespace ezg::gl

#endif  //EASYGRAPHICS_VERTEX_ARRAY_HPP
