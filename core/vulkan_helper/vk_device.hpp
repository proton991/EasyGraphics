#ifndef __VK_DEVICE_H__
#define __VK_DEVICE_H__

#include <cassert>
#include "vk_debug.hpp"
#include "vk_dispatch.hpp"
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
enum class QueueType { graphics = 0, compute = 1, transfer = 2, present = 3 };
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
                      VkQueueFlags excludedBits) const;
  uint32_t GetPresentQFIndex(VkSurfaceKHR surface) const;
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

// For advanced device queue setup
struct CustomQueueDescription {
  explicit CustomQueueDescription(uint32_t index, uint32_t count,
                                  const std::vector<float>& priorities)
      : index(index), count(count), priorities(priorities) {
    assert(priorities.size() == count);
  }
  uint32_t index = 0;
  uint32_t count = 0;
  std::vector<float> priorities;
};

class Device {
public:
  operator VkDevice() const { return vkDevice; }

  uint32_t GetQueueIndex(QueueType type) const;

  VkQueue GetQueue(QueueType type) const;

  // Return a loaded dispatch table
  DispatchTable MakeDispatchTable() const { return {vkDevice, fp_vkGetDeviceProcAddr}; };

  //  void SetupDebugUtil();

  void DisplayInfo() const;

  PhysicalDevice physicalDevice;
  std::vector<VkQueueFamilyProperties> queueFamilies;
  VkDevice vkDevice                              = VK_NULL_HANDLE;
  VkSurfaceKHR surface                           = VK_NULL_HANDLE;
  VkAllocationCallbacks* allocationCallbacks    = VK_NULL_HANDLE;
  PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr = nullptr;
  DispatchTable dispatchTable;

private:
  struct {
    PFN_vkGetDeviceQueue fp_vkGetDeviceQueue = nullptr;
    PFN_vkDestroyDevice fp_vkDestroyDevice   = nullptr;
  } internalTable;
  //  debug::DebugUtil debugUtil;
  friend class DeviceBuilder;
  friend void DestroyDevice(Device device);
};
class DeviceBuilder {
public:
  explicit DeviceBuilder(PhysicalDevice physicalDevice);

  Device Build() const;

  template <typename T>
  DeviceBuilder& addPNext(T* structure) {
    deviceInfo.pNextChain.push_back(reinterpret_cast<VkBaseOutStructure*>(structure));
    return *this;
  }

private:
  PhysicalDevice physicalDevice;
  struct DeviceInfo {
    std::vector<VkBaseOutStructure*> pNextChain;
    VkDeviceCreateFlags flags                  = static_cast<VkDeviceCreateFlags>(0);
    VkAllocationCallbacks* allocationCallbacks = VK_NULL_HANDLE;
  } deviceInfo;
};
}  // namespace vkh
#endif  // __VK_DEVICE_H__