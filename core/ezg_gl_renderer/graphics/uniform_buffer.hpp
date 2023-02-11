#ifndef UNIFORM_BUFFER_HPP
#define UNIFORM_BUFFER_HPP
#include <memory>

namespace ezg::gl {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;

class UniformBuffer {
public:
  static UniformBufferPtr Create(uint32_t size, uint32_t binding) {
    return std::make_shared<UniformBuffer>(size, binding);
  }
  UniformBuffer(uint32_t size, uint32_t binding);
  virtual ~UniformBuffer();

  void set_data(const void* data, uint32_t size, uint32_t offset = 0) const;

private:
  uint32_t m_id{0};
};
}
#endif  //UNIFORM_BUFFER_HPP
