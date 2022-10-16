#include "CreateSwapchain.hpp"
#include <SDL2/SDL_vulkan.h>
#include <string>
#include "vulkan_helper/core.hpp"

namespace egv {
void EGEngine::Init() {
  auto windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
  m_window         = SDL_CreateWindow("EGEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                      m_windowExtent.width, m_windowExtent.height, windowFlags);
  InitVulkan();
  InitSwapchain();
  m_initialized = true;
}
void EGEngine::InitVulkan() {
  // Build instance
  vkh::Instance vkhInstance = vkh::InstanceBuilder()
                                  .EnableValidationLayers(true)
                                  .SetAppName("First App")
                                  .SetApiVersion(1, 3, 0)
                                  .Build();
  m_instance            = vkhInstance.instance;
  m_debugUtilsMessenger = vkhInstance.debugUtilsMessenger;
  SDL_Vulkan_CreateSurface(m_window, m_instance, &m_surface);
  // Select physical device
  vkh::PhysicalDeviceSelector pdSelector{vkhInstance};
  vkh::PhysicalDevice vkhPhysicalDevice =
      pdSelector.SetSurface(m_surface).RequirePresent(true).Select();
  vkh::DeviceBuilder deviceBuilder{vkhPhysicalDevice};
  vkh::Device vkhDevice         = deviceBuilder.Build();
  m_device                      = vkhDevice.vkDevice;
  m_chosenGPU                   = vkhDevice.physicalDevice;
  m_gpuProperties               = vkhPhysicalDevice.properties;
  m_queueFamilyIndices.graphics = vkhDevice.GetQueueIndex(vkh::QueueType::graphics);
  m_queueFamilyIndices.present  = vkhDevice.GetQueueIndex(vkh::QueueType::present);
  m_queueFamilyIndices.compute  = vkhDevice.GetQueueIndex(vkh::QueueType::compute);
  m_queueFamilyIndices.transfer = vkhDevice.GetQueueIndex(vkh::QueueType::transfer);

  m_queueFamilies.graphics = vkhDevice.GetQueue(vkh::QueueType::graphics);
  m_queueFamilies.present  = vkhDevice.GetQueue(vkh::QueueType::present);
  m_queueFamilies.compute  = vkhDevice.GetQueue(vkh::QueueType::compute);
  m_queueFamilies.transfer = vkhDevice.GetQueue(vkh::QueueType::transfer);

  m_dispatchTable = vkhDevice.MakeDispatchTable();
  vkh::VulkanFunction::GetInstance().GetDeviceProcAddr(m_device, fp_vkDestroyDevice,
                                                       "vkDestroyDevice");
}
void EGEngine::InitSwapchain() {
  vkh::SwapchainBuilder swapchainBuilder{m_chosenGPU, m_device, m_surface, m_queueFamilyIndices.graphics, m_queueFamilyIndices.present};
  vkh::Swapchain vkhSwapchain = swapchainBuilder.UseDefaultFormat()
                                    .SetDesiredPresentMode(VK_PRESENT_MODE_FIFO_KHR)
                                    .SetExtent(m_windowExtent.width, m_windowExtent.height)
                                    .Build();
  m_swapchain            = vkhSwapchain.swapchain;
  m_swapchainImageFormat = vkhSwapchain.imageFormat;
//  m_swapchainImages      = vkhSwapchain.GetImages();
//  m_swapchainImageViews  = vkhSwapchain.GetImageViews();
}
void EGEngine::DisplayInfo() {
  std::cout << "\n=========================================\n";
  std::cout << "Displaying Physical Device Info \n";
  std::cout << "Physical Device Name: " << m_gpuProperties.deviceName << "\n";
  std::cout << "Physical Device Type: " << m_gpuProperties.deviceType << "\n";
  std::cout << "Physical Device Api Version: " << m_gpuProperties.apiVersion << "\n";
  std::cout << "Physical Device Driver Version: " << m_gpuProperties.driverVersion << "\n";
  std::cout << "Physical Device Driver VendorID: " << m_gpuProperties.vendorID << "\n";
  vkh::Log("\n=========================================");
  vkh::Log("Displaying Device Queue Info: ");
  std::cout << "Graphics Queue: " << m_queueFamilyIndices.graphics << "\n";
  std::cout << "Transfer Queue: " << m_queueFamilyIndices.transfer << "\n";
  std::cout << "Compute Queue: " << m_queueFamilyIndices.compute << "\n";
  std::cout << "Compute Queue: " << m_queueFamilyIndices.present << "\n";
  vkh::Log("\n=========================================");
}

void EGEngine::Destroy() {
  if (m_initialized) {
    m_dispatchTable.fp_vkDeviceWaitIdle(m_device);
    m_dispatchTable.fp_vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkh::VulkanFunction::GetInstance().fp_vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    fp_vkDestroyDevice(m_device, nullptr);
    vkh::DestroyDebugUtilsMessenger(m_instance, m_debugUtilsMessenger, nullptr);
    vkh::VulkanFunction::GetInstance().fp_vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);
  }
}
}  // namespace egv
