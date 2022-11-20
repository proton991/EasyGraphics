#ifndef COMMON_HPP
#define COMMON_HPP

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace spd = spdlog;
namespace ezg::vk {

//template <typename T>
//void GetInstanceProcAddr(VkInstance instance, T& out_ptr, const char* func_name) {
//  out_ptr = reinterpret_cast<T>(Context::InstanceFuncLoader()(instance, func_name));
//}
//
//template <typename T>
//void GetDeviceProcAddr(VkDevice device, T& out_ptr, const char* func_name) {
//  out_ptr = reinterpret_cast<T>(Context::DeviceFuncLoader()(device, func_name));
//}

enum QueueIndices {
  QUEUE_INDEX_GRAPHICS,
  QUEUE_INDEX_COMPUTE,
  QUEUE_INDEX_TRANSFER,
  QUEUE_INDEX_VIDEO_DECODE,
  QUEUE_INDEX_COUNT
};

template <typename T>
void SetPNextChain(T& structure, const std::vector<VkBaseOutStructure*>& structs) {
  structure.pNext = nullptr;
  if (structs.empty())
    return;
  for (uint32_t i = 0; i < structs.size() - 1; i++) {
    structs.at(i)->pNext = structs.at(i + 1);
  }
  structure.pNext = structs.at(0);
}

const char* ToString(const VkResult result);

void Check(VkResult result, const char* operation);
}  // namespace ezg::vk

#endif  //COMMON_HPP
