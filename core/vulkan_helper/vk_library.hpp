#ifndef VK_LIBRARY_HPP
#define VK_LIBRARY_HPP

#include <vulkan/vulkan.h>

#if defined(_WIN32)
#include <fcntl.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif  // _WIN32

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

namespace vkh {
class VulkanLibrary {
public:
  static VulkanLibrary& GetInstance();

  void Close();

  template <typename T>
  void LoadFunc(T& func_dest, const char* func_name);


private:
  VulkanLibrary();
  PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = VK_NULL_HANDLE;
  friend class VulkanFunction;

#if defined(__linux__) || defined(__APPLE__)
  void* library;
#elif defined(_WIN32)
  HMODULE library;
#endif
};
template <typename T>
void VulkanLibrary::LoadFunc(T& func_dest, const char* func_name) {
#if defined(__linux__) || defined(__APPLE__)
  func_dest = reinterpret_cast<T>(dlsym(library, func_name));
#elif defined(_WIN32)
  func_dest = reinterpret_cast<T>(GetProcAddress(library, func_name));
#endif
}

}  // namespace vkh
#endif  //VK_LIBRARY_HPP
