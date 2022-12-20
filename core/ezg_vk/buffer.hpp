#ifndef EASYGRAPHICS_BUFFER_HPP
#define EASYGRAPHICS_BUFFER_HPP
#include <string>
#include "vulkan_base.hpp"
#include "common.hpp"
namespace ezg::vk {
class Device;
struct BufferCreateInfo {
  std::string name;
  VkDeviceSize size;
  VkBufferUsageFlags bufferUsage;
  VmaAllocationCreateFlags vmaAllocFlags;
  VmaMemoryUsage vmaMemoryUsage{VMA_MEMORY_USAGE_AUTO};
};

class Buffer {
public:
  NO_COPY(Buffer)
  NO_MOVE(Buffer)

  Buffer(const Device& device, const BufferCreateInfo& info);

  ~Buffer();

  [[nodiscard]] VkBuffer Handle() const { return m_buffer; }
private:
  const Device& m_device;
  const std::string m_name;
  VkBuffer m_buffer{};
  VmaAllocation m_allocation{};
  VkDeviceSize m_size{};
};
}  // namespace ezg::vk

#endif  //EASYGRAPHICS_BUFFER_HPP
