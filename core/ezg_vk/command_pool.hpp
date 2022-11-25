#ifndef EASYGRAPHICS_COMMAND_POOL_HPP
#define EASYGRAPHICS_COMMAND_POOL_HPP

#include <vector>
#include <unordered_set>
#include "common.hpp"
#include "volk.h"

namespace ezg::vk {
class Device;
class CommandPool {
public:
  NO_COPY(CommandPool)
  CommandPool(const Device* device, uint32_t qFamilyIndex);
  ~CommandPool();

  CommandPool(CommandPool&& ) noexcept;
  CommandPool& operator=(CommandPool &&) noexcept;

  void Begin();
  void Trim();
  VkCommandBuffer RequestCmdBuffer();
  void SignalSubmitted(VkCommandBuffer cmd);

private:
  const Device* m_device;
  VkCommandPool m_pool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> m_buffers;
  std::unordered_set<VkCommandBuffer> m_inFlight;
  unsigned m_index{0};
};
}
#endif  //EASYGRAPHICS_COMMAND_POOL_HPP
