#ifndef UNIFORM_BUFFER_HPP
#define UNIFORM_BUFFER_HPP
#include "base.hpp"

namespace ezg::gl {

class UniformBuffer {
public:
  static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding) {
    return CreateRef<UniformBuffer>(size, binding);
  }
  UniformBuffer(uint32_t size, uint32_t binding);
  virtual ~UniformBuffer();

  void set_data(const void* data, uint32_t size, uint32_t offset = 0) const;

private:
  uint32_t m_id{0};
};
}
#endif  //UNIFORM_BUFFER_HPP
