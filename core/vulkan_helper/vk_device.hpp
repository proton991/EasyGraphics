#ifndef __VK_DEVICE_H__
#define __VK_DEVICE_H__

#include "vk_instance.hpp"
namespace vkh {
// Sentinel value, used in implementation only
const uint32_t QUEUE_INDEX_MAX_VALUE = 65536;
class PhysicalDeviceSelector;
class DeviceBuilder;
using PhysicalDeviceSelectorRef = PhysicalDeviceSelector&;
enum class PreferredDeviceType {
  other       = 0,
  integrated  = 1,
  discrete    = 2,
  virtual_gpu = 3,
  cpu         = 4
};

struct PhysicalDevice {
  std::string pdName;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkSurfaceKHR surface            = VK_NULL_HANDLE;

  // Note that this reflects selected features carried over from required features, not all features the physical device supports.
  VkPhysicalDeviceFeatures features{};
  VkPhysicalDeviceProperties properties{};
  VkPhysicalDeviceMemoryProperties memoryProperties{};

  // A conversion function which allows this PhysicalDevice to be used
  // in places where VkPhysicalDevice would have been used.
  operator VkPhysicalDevice() const { return this->physicalDevice; }
  std::vector<std::string> GetExtensions() const { return this->extensions; }
  std::vector<VkQueueFamilyProperties> GetQueueFamilies() const { return queueFamilies; }
  uint32_t GetQFIndex(const std::string& name, VkQueueFlags requiredBits,
                      VkQueueFlags excludedBits);
  uint32_t GetPresentQFIndex(VkSurfaceKHR surface);
  bool CheckExtensionsSupported(const std::vector<std::string>& exts);
  void DisplayInfo();

private:
  std::vector<std::string> extensions;
  std::vector<VkQueueFamilyProperties> queueFamilies;
  friend class PhysicalDeviceSelector;
  friend class DeviceBuilder;
};

class PhysicalDeviceSelector {
public:
  explicit PhysicalDeviceSelector(const Instance& instance);
  PhysicalDevice Select();
  PhysicalDeviceSelectorRef AddRequiredExtension(const char* extension);
  PhysicalDeviceSelectorRef AddRequiredExtension(std::vector<const char*>& extensions);
  PhysicalDeviceSelectorRef SetSurface(VkSurfaceKHR surface);
  PhysicalDeviceSelectorRef SetRequiredFeatures(const VkPhysicalDeviceFeatures& features);
  PhysicalDeviceSelectorRef RequirePresent(bool flag);
  PhysicalDeviceSelectorRef PreferGPUType(PreferredDeviceType type);

private:
  PhysicalDevice PopulateDetails(VkPhysicalDevice vkpd);
  bool IsSuitable(PhysicalDevice pd);
  struct InstanceInfo {
    VkInstance instance  = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
  } instanceInfo;
  struct SelectionCriteria {
    PreferredDeviceType pdType = PreferredDeviceType::discrete;
    bool requirePresent        = true;
    // dedicate compute and transfer queue
    bool requireDedicateComputeQueue  = false;
    bool requireDedicateTransferQueue = true;
    std::vector<std::string> requiredExtensions;
    VkPhysicalDeviceFeatures requiredFeatures;
  } criteria;
};
}  // namespace vkh
#endif  // __VK_DEVICE_H__