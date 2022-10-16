#ifndef CREATE_SWAPCHAIN_HPP
#define CREATE_SWAPCHAIN_HPP
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "vulkan_helper/vk_device.hpp"
#include "vulkan_helper/vk_dispatch.hpp"
namespace egv {
class EGEngine {
public:
  void Init();
  void Destroy();
  void DisplayInfo();

private:
  void InitVulkan();
  void InitSwapchain();

  bool m_initialized{false};
  //  std::unique_ptr<vkh::Instance> m_vkhInstance;
  //  std::unique_ptr<vkh::Device> m_vkhDevice;
  VkInstance m_instance;
  VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
  VkPhysicalDevice m_chosenGPU;
  PFN_vkDestroyDevice fp_vkDestroyDevice = nullptr;
  VkPhysicalDeviceProperties m_gpuProperties;
  VkDevice m_device;
  VkSurfaceKHR m_surface;
  SDL_Window* m_window{nullptr};
  VkExtent2D m_windowExtent{1600, 900};
  struct {
    uint32_t graphics;
    uint32_t present;
    uint32_t compute;
    uint32_t transfer;
  } m_queueFamilyIndices;
  struct {
    VkQueue graphics;
    VkQueue present;
    VkQueue compute;
    VkQueue transfer;
  } m_queueFamilies;

  VkSwapchainKHR m_swapchain;
  VkFormat m_swapchainImageFormat;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;
  vkh::DispatchTable m_dispatchTable;
};
}  // namespace egv

#endif  //CREATE_SWAPCHAIN_HPP
