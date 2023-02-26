#ifndef EASYGRAPHICS_BUFFER_HPP
#define EASYGRAPHICS_BUFFER_HPP
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "base.hpp"

namespace ezg::gl {
enum class BufferDataType {
  None = 0,
  Float,
  Vec2f,
  Vec3f,
  Vec4f,
  Mat2f,
  Mat3f,
  Mat4f,
  Int,
  Vec2i,
  Vec3i,
  Vec4i
};
static size_t data_type_size(BufferDataType type) {
  switch (type) {
    case BufferDataType::Float:
      return sizeof(float);
    case BufferDataType::Vec2f:
      return sizeof(float) * 2;
    case BufferDataType::Vec3f:
      return sizeof(float) * 3;
    case BufferDataType::Vec4f:
      return sizeof(float) * 4;
    case BufferDataType::Mat2f:
      return sizeof(float) * 4 * 4;
    case BufferDataType::Mat3f:
      return sizeof(float) * 4 * 9;
    case BufferDataType::Mat4f:
      return sizeof(float) * 4 * 16;
    case BufferDataType::Int:
      return sizeof(int);
    case BufferDataType::Vec2i:
      return sizeof(int) * 2;
    case BufferDataType::Vec3i:
      return sizeof(int) * 3;
    case BufferDataType::Vec4i:
      return sizeof(int) * 4;
    default:
      return 0;
  }

  return 0;
}

static uint32_t data_type_count(BufferDataType type) {
  switch (type) {
    case BufferDataType::Float:
    case BufferDataType::Int:
      return 1;
    case BufferDataType::Vec2f:
    case BufferDataType::Vec2i:
      return 2;
    case BufferDataType::Vec3f:
    case BufferDataType::Vec3i:
      return 3;
    case BufferDataType::Vec4f:
    case BufferDataType::Vec4i:
    case BufferDataType::Mat2f:
      return 4;
    case BufferDataType::Mat3f:
      return 9;
    case BufferDataType::Mat4f:
      return 16;
    default:
      return 0;
  }

  return 0;
}

struct BufferElement {
  BufferElement(std::string name_, BufferDataType type_, bool normalized_ = false)
      : name(std::move(name_)), type(type_), size(data_type_size(type)), count(data_type_count(type)),normalized(normalized_) {}
  std::string name;
  BufferDataType type;
  size_t size;
  size_t offset;
  uint32_t count;
  bool normalized{false};
};

class BufferView {
public:
  BufferView() = default;
  BufferView(std::initializer_list<BufferElement> elements) : m_elements(elements) {
    calc_stride_and_offset();
  }

  auto get_stride() const { return m_stride; }

  [[nodiscard]] const std::vector<BufferElement>& get_elements() const { return m_elements; }
private:
  void calc_stride_and_offset();
  uint32_t m_stride{0};
  std::vector<BufferElement> m_elements;
};

class VertexBuffer {
public:
  static Ref<VertexBuffer> Create(uint32_t size, const void* data) {
    return CreateRef<VertexBuffer>(size, data);
  }
  explicit VertexBuffer(uint32_t size);
  VertexBuffer(uint32_t size, const void* data);

  ~VertexBuffer();

  void bind() const;
  void unbind() const;

  void set_buffer_view(const BufferView& buffer_view) { m_buffer_view = buffer_view; }
  void set_data(uint32_t size, const void* data);

  [[nodiscard]] const BufferView& get_buffer_view() const { return m_buffer_view; }
private:
  uint32_t m_id{0};
  BufferView m_buffer_view;
};

class IndexBuffer {
public:
  static Ref<IndexBuffer> Create(uint32_t count, const void* data) {
    return CreateRef<IndexBuffer>(count, data);
  }
  IndexBuffer(uint32_t count, const void* data);
  ~IndexBuffer();

  uint32_t get_count() const { return m_count; }

  void bind() const;
  void unbind() const;

private:
  uint32_t m_id{0};
  uint32_t m_count{0};
};

}  // namespace ezg::gl
#endif  //EASYGRAPHICS_BUFFER_HPP
