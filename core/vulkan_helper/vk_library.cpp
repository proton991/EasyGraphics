#include "vk_library.hpp"

namespace vkh {
VulkanLibrary::VulkanLibrary() {
#if defined(__linux__)
  library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
  if (!library)
    library = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
#elif defined(__APPLE__)
  library = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
  if (!library)
    library = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
#elif defined(_WIN32)
  library = LoadLibrary(TEXT("vulkan-1.dll"));
#else
  assert(false && "Unsupported platform");
#endif
  if (!library)
    return;
  LoadFunc(fp_vkGetInstanceProcAddr, "vkGetInstanceProcAddr");
}

void VulkanLibrary::Close() {
#if defined(__linux__) || defined(__APPLE__)
  dlclose(library);
#elif defined(_WIN32)
  FreeLibrary(library);
#endif
  library = nullptr;
}
VulkanLibrary& VulkanLibrary::GetInstance() {
  static VulkanLibrary vulkanLibrary;
  return vulkanLibrary;
}
}  // namespace vkh