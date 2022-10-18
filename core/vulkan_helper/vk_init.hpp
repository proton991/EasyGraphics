#ifndef VK_INIT_HPP
#define VK_INIT_HPP
#include <vulkan/vulkan.h>

namespace vkh::init {
VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t qFIndex, VkCommandPoolCreateFlags flags);

VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count /*= 1*/, VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/);

}  // namespace vkh::init
#endif  //VK_INIT_HPP
