#include "vulkan_helper/core.hpp"
#include <cassert>
int main(int argc, char* argv[]) {
  vkh::Instance vkhInstance = vkh::InstanceBuilder()
                                  .EnableValidationLayers(true)
                                  .SetAppName("First App")
                                  .SetApiVersion(1, 3, 0)
                                  .Build();
  vkh::SystemInfo::GetInstance().DisplayInfo();
  assert(vkhInstance.instance != nullptr);
  vkh::Log("\nInstance enabled extensions: ");
  vkh::Log(vkhInstance.enabledExtensions);
  vkh::Log("\nInstance enabled layers: ");
  vkh::Log(vkhInstance.enabledLayers);

  vkh::PhysicalDeviceSelector pdSelector{vkhInstance};
  vkh::PhysicalDevice pd = pdSelector.RequirePresent(false).Select();
  pd.DisplayInfo();
}
