#ifndef COMMON_HPP
#define COMMON_HPP

#include <vulkan/vulkan.h>
#include "context.hpp"

namespace ezg::vk {

template <typename T>
void GetInstanceProcAddr(VkInstance instance, T& out_ptr, const char* func_name) {
  out_ptr = reinterpret_cast<T>(Context::InstanceFuncLoader()(instance, func_name));
}

template <typename T>
void GetDeviceProcAddr(VkDevice device, T& out_ptr, const char* func_name) {
  out_ptr = reinterpret_cast<T>(Context::DeviceFuncLoader()(device, func_name));
}

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
}
#endif  //COMMON_HPP
