#include "command_pool.hpp"
#include "device.hpp"

namespace ezg::vk {
CommandPool::CommandPool(const Device* device, uint32_t qFamilyIndex) : m_device(device) {
  VkCommandPoolCreateInfo info{};
  info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  info.queueFamilyIndex = qFamilyIndex;
  if (qFamilyIndex != VK_QUEUE_FAMILY_IGNORED) {
    vkCreateCommandPool(m_device->Handle(), &info, nullptr, &m_pool);
  }
}

CommandPool::~CommandPool() {
  if (!m_buffers.empty()) {
    vkFreeCommandBuffers(m_device->Handle(), m_pool, m_buffers.size(), m_buffers.data());
  }
  if (m_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_device->Handle(), m_pool, nullptr);
  }
}

CommandPool::CommandPool(CommandPool&& other) noexcept {
  *this = std::move(other);
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept {
  if (this != &other) {
    m_device = other.m_device;
    if (!m_buffers.empty()) {
      vkFreeCommandBuffers(m_device->Handle(), m_pool, m_buffers.size(), m_buffers.data());
    }
    if (m_pool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(m_device->Handle(), m_pool, nullptr);
    }

    m_pool = VK_NULL_HANDLE;
    m_buffers.clear();
    std::swap(m_pool, other.m_pool);
    std::swap(m_buffers, other.m_buffers);

    this->m_index = other.m_index;
    other.m_index = 0;

    m_inFlight.clear();
    std::swap(m_inFlight, other.m_inFlight);
  }
  return *this;
}

void CommandPool::Begin() {
  if (m_pool == VK_NULL_HANDLE) {
    return;
  }
  if (m_index > 0) {
    vkResetCommandPool(m_device->Handle(), m_pool, 0);
  }
  m_index = 0;
}

void CommandPool::Trim() {
  if (m_pool == VK_NULL_HANDLE) {
    return;
  }
  vkResetCommandPool(m_device->Handle(), m_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
  if (!m_buffers.empty()) {
    vkFreeCommandBuffers(m_device->Handle(), m_pool, m_buffers.size(), m_buffers.data());
  }
  if (m_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_device->Handle(), m_pool, nullptr);
  }
  m_buffers.clear();
  vkTrimCommandPool(m_device->Handle(), m_pool, 0);
}

VkCommandBuffer CommandPool::RequestCmdBuffer() {
  if (m_index < m_buffers.size()) {
    auto ret = m_buffers[m_index++];
    m_inFlight.insert(ret);
    return ret;
  } else {
    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo info{};
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool        = m_pool;
    info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

    vkAllocateCommandBuffers(m_device->Handle(), &info, &cmd);
    m_inFlight.insert(cmd);
    m_buffers.push_back(cmd);
    m_index++;
    return cmd;
  }
}

void CommandPool::SignalSubmitted(VkCommandBuffer cmd) {
  m_inFlight.erase(cmd);
}

}  // namespace ezg::vk