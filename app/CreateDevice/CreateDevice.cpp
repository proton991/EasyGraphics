#include "vulkan_helper/core.hpp"
#include <cassert>
int main(int argc, char* argv[]) {
  vkh::Instance vkhInstance = vkh::InstanceBuilder()
                                  .EnableValidationLayers(true)
                                  .SetAppName("First App")
                                  .SetApiVersion(1, 3, 0)
                                  .Build();
  vkh::SystemInfo::GetInstance().DisplayInfo();
  assert(vkhInstance.instance != VK_NULL_HANDLE);
  assert(vkhInstance.debugMessenger != VK_NULL_HANDLE);
//  vkh::Log("\nInstance enabled extensions: ");
//  vkh::Log(vkhInstance.enabledExtensions);
//  vkh::Log("\nInstance enabled layers: ");
//  vkh::Log(vkhInstance.enabledLayers);

  vkh::PhysicalDeviceSelector pdSelector{vkhInstance};
  vkh::PhysicalDevice pd = pdSelector.RequirePresent(false).Select();
//  pd.DisplayInfo();
  vkh::DeviceBuilder deviceBuilder(pd, vkhInstance);
  vkh::Device vkhDevice = deviceBuilder.Build();
  vkh::DispatchTable dispatchTable = vkhDevice.MakeDispatchTable();
//  device.DisplayInfo();
  dispatchTable.fp_vkDeviceWaitIdle(vkhDevice.vkDevice);
//  vkh::DestroyDevice(vkhDevice);
  vkh::DestroyDebugUtilsMessenger(vkhInstance.instance, vkhInstance.debugMessenger, nullptr);
//  vkh::DestroyInstance(vkhInstance);
}
