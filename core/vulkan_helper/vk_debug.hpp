#ifndef VK_DEBUG_HPP
#define VK_DEBUG_HPP
#include "vk_instance.hpp"
#include "vk_tools.hpp"
namespace vkh::debug {
class DebugUtil final {
public:
  //  VULKAN_NON_COPIABLE(DebugUtil)

  explicit DebugUtil(const class Instance& instance);
  ~DebugUtil() = default;

  void SetDevice(VkDevice device) { this->device = device; }

  void SetObjectName(const VkAccelerationStructureKHR& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);
  }
  void SetObjectName(const VkBuffer& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_BUFFER);
  }
  void SetObjectName(const VkCommandBuffer& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_COMMAND_BUFFER);
  }
  void SetObjectName(const VkDescriptorSet& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET);
  }
  void SetObjectName(const VkDescriptorSetLayout& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
  }
  void SetObjectName(const VkDeviceMemory& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_DEVICE_MEMORY);
  }
  void SetObjectName(const VkFramebuffer& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_FRAMEBUFFER);
  }
  void SetObjectName(const VkImage& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_IMAGE);
  }
  void SetObjectName(const VkImageView& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_IMAGE_VIEW);
  }
  void SetObjectName(const VkPipeline& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_PIPELINE);
  }
  void SetObjectName(const VkQueue& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_QUEUE);
  }
  void SetObjectName(const VkRenderPass& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_RENDER_PASS);
  }
  void SetObjectName(const VkSemaphore& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_SEMAPHORE);
  }
  void SetObjectName(const VkShaderModule& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_SHADER_MODULE);
  }
  void SetObjectName(const VkSwapchainKHR& object, const char* name) const {
    SetObjectName(object, name, VK_OBJECT_TYPE_SWAPCHAIN_KHR);
  }

private:
  template <typename T>
  void SetObjectName(const T& object, const char* name, VkObjectType type) const {
#ifndef NDEBUG
    VkDebugUtilsObjectNameInfoEXT info = {};
    info.sType                         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.pNext                         = nullptr;
    info.objectHandle                  = reinterpret_cast<const uint64_t&>(object);
    info.objectType                    = type;
    info.pObjectName                   = name;

    VkCheck(fp_vkSetDebugUtilsObjectNameEXT(device, &info), "set object name");
#endif
  }

  PFN_vkSetDebugUtilsObjectNameEXT fp_vkSetDebugUtilsObjectNameEXT;

  VkDevice device{};
};

VkResult CreateDebugUtilsMessenger(VkInstance instance,
                                   PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
                                   VkDebugUtilsMessageSeverityFlagsEXT severity,
                                   VkDebugUtilsMessageTypeFlagsEXT type, void* user_data_pointer,
                                   VkDebugUtilsMessengerEXT* pDebugMessenger,
                                   VkAllocationCallbacks* allocation_callbacks);

}  // namespace vkh::debug
#endif  //VK_DEBUG_HPP
