#include "vk_device.hpp"
#include <memory>
#include <string>
#include "vk_functions.hpp"
namespace vkh {
uint32_t PhysicalDevice::GetQFIndex(const std::string& name, const VkQueueFlags requiredBits,
                                    const VkQueueFlags excludedBits) const {
  const auto family =
      std::find_if(queueFamilies.begin(), queueFamilies.end(),
                   [requiredBits, excludedBits](const VkQueueFamilyProperties& queueFamily) {
                     return queueFamily.queueCount > 0 && queueFamily.queueFlags & requiredBits &&
                            !(queueFamily.queueFlags & excludedBits);
                   });

  if (family == queueFamilies.end()) {
    return QUEUE_INDEX_MAX_VALUE;
  }

  return static_cast<uint32_t>(family - queueFamilies.cbegin());
}

uint32_t PhysicalDevice::GetPresentQFIndex(VkSurfaceKHR surface) const {
  for (uint32_t i = 0; i < queueFamilies.size(); i++) {
    VkBool32 presentSupport = false;
    if (surface != VK_NULL_HANDLE) {
      VkCheck(VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceSurfaceSupportKHR(
                  physicalDevice, i, surface, &presentSupport),
              "Get present support");
      return queueFamilies[i].queueCount > 0 && presentSupport;
    }
  }
  return QUEUE_INDEX_MAX_VALUE;
}

bool PhysicalDevice::CheckExtensionsSupported(const std::vector<std::string>& exts) {
  int count = 0;
  for (const auto& ext : extensions) {
    for (const auto& requiredExt : exts) {
      if (ext == requiredExt) {
        count++;
      }
    }
  }
  return count == exts.size();
}

PhysicalDeviceSelector::PhysicalDeviceSelector(const Instance& instance) {
  instanceInfo.instance = instance;
}
PhysicalDevice PhysicalDeviceSelector::Select() {
  std::vector<VkPhysicalDevice> vkpds = GetVector<VkPhysicalDevice>(
      VulkanFunction::GetInstance().fp_vkEnumeratePhysicalDevices, instanceInfo.instance);
  //  GetEnumerateVector(instanceInfo.instance,
  //                     VulkanFunction::GetInstance().fp_vkEnumeratePhysicalDevices, vkpds);
  if (vkpds.empty()) {
    throw std::runtime_error("No physical devices available!");
  }

  auto fill_out_phys_dev_with_criteria = [&](PhysicalDevice& pd) {
    pd.features = criteria.requiredFeatures;
    pd.extensions.clear();
    pd.extensions.insert(pd.extensions.end(), criteria.requiredExtensions.begin(),
                         criteria.requiredExtensions.end());
    //    if (portability_ext_available) {
    //      pd.extensions.push_back("VK_KHR_portability_subset");
    //    }
  };
  // Return the first suitable physical device

  for (auto& vkpd : vkpds) {
    PhysicalDevice pd = PopulateDetails(vkpd);
    if (IsSuitable(pd)) {
      fill_out_phys_dev_with_criteria(pd);
      return pd;
    }
  }
  return {};
}
void PhysicalDevice::DisplayInfo() {
  std::cout << "\n=========================================\n";
  std::cout << "Displaying Physical Device Info \n";
  std::cout << "Physical Device Name: " << pdName << "\n";
  std::cout << "Physical Device Type: " << properties.deviceType << "\n";
  std::cout << "Physical Device Api Version: " << properties.apiVersion << "\n";
  std::cout << "Physical Device Driver Version: " << properties.driverVersion << "\n";
  std::cout << "Physical Device Driver VendorID: " << properties.vendorID << "\n";
  std::cout << "=========================================\n";
  std::cout << "Enabled Extensions are: \n";
  Log(extensions);
}
PhysicalDevice PhysicalDeviceSelector::PopulateDetails(VkPhysicalDevice vkpd) {
  PhysicalDevice pd{};
  pd.physicalDevice        = vkpd;
  pd.surface               = instanceInfo.surface;
  const auto queueFamilies = GetVector<VkQueueFamilyProperties>(
      VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceQueueFamilyProperties, vkpd);

  VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceProperties(vkpd, &pd.properties);
  VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceFeatures(vkpd, &pd.features);
  VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceMemoryProperties(vkpd, &pd.memoryProperties);
  pd.queueFamilies = GetVector<VkQueueFamilyProperties>(
      VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceQueueFamilyProperties, vkpd);
  pd.pdName = pd.properties.deviceName;
  //  const auto availableExtensions = GetEnumerateVector(vkpd, static_cast<const char*>(nullptr),
  const auto availableExtensions = GetVector<VkExtensionProperties>(
      VulkanFunction::GetInstance().fp_vkEnumerateDeviceExtensionProperties, vkpd, nullptr);
  for (const auto& ext : availableExtensions) {
    pd.extensions.emplace_back(ext.extensionName);
  }
  return pd;
}
bool PhysicalDeviceSelector::IsSuitable(PhysicalDevice pd) {
  bool suitable = true;
  // TODO: Check physical device properties.apiVersion

  // Check dedicate queue support
  if (criteria.requireDedicateComputeQueue &&
      pd.GetQFIndex("compute", VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT) ==
          QUEUE_INDEX_MAX_VALUE) {
    return false;
  }
  if (criteria.requireDedicateTransferQueue &&
      pd.GetQFIndex("transfer", VK_QUEUE_TRANSFER_BIT,
                    VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) == QUEUE_INDEX_MAX_VALUE) {
    return false;
  }
  // Check graphics queue support
  if (pd.GetQFIndex("graphic", VK_QUEUE_GRAPHICS_BIT, 0) == QUEUE_INDEX_MAX_VALUE) {
    return false;
  }
  // Check present queue support if required
  if (criteria.requirePresent &&
      pd.GetPresentQFIndex(instanceInfo.surface) == QUEUE_INDEX_MAX_VALUE) {
    return false;
  }
  // Check required extensions support
  if (!pd.CheckExtensionsSupported(criteria.requiredExtensions)) {
    return false;
  }
  // TODO: Check required features support

  return suitable;
}
PhysicalDeviceSelectorRef PhysicalDeviceSelector::AddRequiredExtension(const char* extension) {
  criteria.requiredExtensions.emplace_back(extension);
  return *this;
}
PhysicalDeviceSelectorRef PhysicalDeviceSelector::AddRequiredExtension(
    std::vector<const char*>& extensions) {
  criteria.requiredExtensions.insert(criteria.requiredExtensions.end(), extensions.cbegin(),
                                     extensions.cend());
  return *this;
}
PhysicalDeviceSelectorRef PhysicalDeviceSelector::SetSurface(VkSurfaceKHR surface) {
  instanceInfo.surface = surface;
  return *this;
}
PhysicalDeviceSelectorRef PhysicalDeviceSelector::SetRequiredFeatures(
    const VkPhysicalDeviceFeatures& features) {
  criteria.requiredFeatures = features;
  return *this;
}
PhysicalDeviceSelectorRef PhysicalDeviceSelector::RequirePresent(bool flag) {
  criteria.requirePresent = flag;
  return *this;
}
PhysicalDeviceSelectorRef PhysicalDeviceSelector::PreferGPUType(PreferredDeviceType type) {
  criteria.pdType = type;
  return *this;
}

uint32_t Device::GetQueueIndex(QueueType type) const {
  uint32_t index = QUEUE_INDEX_MAX_VALUE;
  std::string name;
  switch (type) {
    case QueueType::graphics:
      index = physicalDevice.GetQFIndex("graphic", VK_QUEUE_GRAPHICS_BIT, 0);
      name  = "graphic";
      break;
    case QueueType::present:
      index = physicalDevice.GetPresentQFIndex(surface);
      name  = "present";
      break;
    case QueueType::compute:
      index = physicalDevice.GetQFIndex("compute", VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
      name  = "compute";
      break;
    case QueueType::transfer:
      index = physicalDevice.GetQFIndex("transfer", VK_QUEUE_TRANSFER_BIT,
                                        VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
      name  = "transfer";
      break;
  }
  if (index == QUEUE_INDEX_MAX_VALUE) {
    throw std::runtime_error(name + " queue not found!");
  }
  return index;
}

VkQueue Device::GetQueue(QueueType type) const {
  auto index = GetQueueIndex(type);
  VkQueue queue;
  internalTable.fp_vkGetDeviceQueue(vkDevice, index, 0, &queue);
  return queue;
}

//void Device::SetupDebugUtil() {
//  debugUtil.SetDevice(vkDevice);
//}

void Device::DisplayInfo() const {
  Log("\n=========================================");
  Log("Displaying Device Queue Info: ");
  Log("Queue Family Size: " + std::to_string(queueFamilies.size()));
  auto graphicsQueueIndex = GetQueueIndex(QueueType::graphics);
  auto transferQueueIndex = GetQueueIndex(QueueType::transfer);
  auto computeQueueIndex  = GetQueueIndex(QueueType::compute);
  std::cout << "Graphics Queue: " << graphicsQueueIndex << "\n";
  std::cout << "Transfer Queue: " << transferQueueIndex << "\n";
  std::cout << "Compute Queue: " << computeQueueIndex << "\n";
}

DeviceBuilder::DeviceBuilder(PhysicalDevice pd)
    : physicalDevice(pd){}

Device DeviceBuilder::Build() const {
  std::vector<CustomQueueDescription> queueDescriptions;
  for (uint32_t i = 0; i < physicalDevice.queueFamilies.size(); i++) {
    queueDescriptions.push_back(CustomQueueDescription{i, 1, std::vector<float>{1.0f}});
  }
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  for (auto& description : queueDescriptions) {
    VkDeviceQueueCreateInfo createInfo = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    createInfo.queueFamilyIndex        = description.index;
    createInfo.queueCount              = description.count;
    createInfo.pQueuePriorities        = description.priorities.data();
    queueCreateInfos.push_back(createInfo);
  }
  std::vector<const char*> extensions;
  for (const auto& ext : physicalDevice.extensions) {
    extensions.push_back(ext.c_str());
  }
  if (physicalDevice.surface != VK_NULL_HANDLE) {
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }
  VkDeviceCreateInfo deviceCreateInfo = {};
  //  deviceCreateInfo.pEnabledFeatures = &physicalDevice.features;
  deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.flags                   = deviceInfo.flags;
  deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
  deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
  Device device;
  VulkanFunction::GetInstance().fp_vkCreateDevice(physicalDevice.physicalDevice, &deviceCreateInfo,
                                                  nullptr, &device.vkDevice);
  device.physicalDevice         = physicalDevice;
  device.surface                = physicalDevice.surface;
  device.queueFamilies          = physicalDevice.queueFamilies;
  device.fp_vkGetDeviceProcAddr = VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr;

  VulkanFunction::GetInstance().GetDeviceProcAddr(
      device.vkDevice, device.internalTable.fp_vkGetDeviceQueue, "vkGetDeviceQueue");
  VulkanFunction::GetInstance().GetDeviceProcAddr(
      device.vkDevice, device.internalTable.fp_vkDestroyDevice, "vkDestroyDevice");
  //  device.SetupDebugUtil();
  return device;
}
}  // namespace vkh