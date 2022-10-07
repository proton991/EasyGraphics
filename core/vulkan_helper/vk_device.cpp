#include "vk_device.hpp"
#include "vk_functions.hpp"
namespace vkh {
uint32_t PhysicalDevice::GetQFIndex(const std::string& name, const VkQueueFlags requiredBits,
                                    const VkQueueFlags excludedBits) {
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

uint32_t PhysicalDevice::GetPresentQFIndex(VkSurfaceKHR surface) {
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
  pd.queueFamilies = GetVector<VkQueueFamilyProperties>(VulkanFunction::GetInstance().fp_vkGetPhysicalDeviceQueueFamilyProperties, vkpd);
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
}  // namespace vkh