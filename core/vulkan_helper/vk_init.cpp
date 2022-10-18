#include "vk_init.hpp"

namespace vkh::init {
VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t qFIndex, VkCommandPoolCreateFlags flags) {
  VkCommandPoolCreateInfo commandPoolCreateInfo = {};
  commandPoolCreateInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.queueFamilyIndex        = qFIndex;
  commandPoolCreateInfo.flags                   = flags;
  commandPoolCreateInfo.pNext                   = nullptr;
  return commandPoolCreateInfo;
}
VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count,
                                                      VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo info = {};
  info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandBufferCount          = count;
  info.commandPool                 = pool;
  info.level                       = level;
  info.pNext = nullptr;
  return info;
}
}  // namespace vkh::init