#include "buffer.hpp"
#include "device.hpp"

namespace ezg::vk {
Buffer::Buffer(const Device& device, const BufferCreateInfo& info) : m_device(device), m_name(info.name) {
  VkBufferCreateInfo bufferInfo = {.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                   .pNext       = nullptr,
                                   .size        = info.size,
                                   .usage       = info.bufferUsage,
                                   .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  VmaAllocationCreateInfo vmaAllocInfo = {.flags = info.vmaAllocFlags,
                                          .usage = info.vmaMemoryUsage};

  Check(vmaCreateBuffer(m_device.Allocator(), &bufferInfo, &vmaAllocInfo, &m_buffer, &m_allocation,
                        nullptr),
        "vma create buffer");

  vmaSetAllocationName(m_device.Allocator(), m_allocation, m_name.c_str());
}

Buffer::~Buffer() {
  vmaDestroyBuffer(m_device.Allocator(), m_buffer, m_allocation);
}

}  // namespace ezg::vk