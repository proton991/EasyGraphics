#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <deque>
#include <functional>
#include <memory>

#include "vulkan_helper/vk_device.hpp"
#include "vulkan_helper/vk_dispatch.hpp"
#include "vulkan_helper/vk_pipeline.hpp"

namespace ege {
struct DestructionQueue {
  std::deque<std::function<void()>> destructors;

  template <typename F>
  void PushFunction(F&& function) {
    static_assert(sizeof(F) < 200, "DONT CAPTURE TOO MUCH IN THE LAMBDA");
    destructors.push_back(function);
  }

  void Flush() {
    for (auto it = destructors.rbegin(); it != destructors.rend(); it++) {
      (*it)();  // call function to free resources
    }
    destructors.clear();
  }
};

class EGEngine {
public:
  void Init();

  void Run();

  void Destroy();

  void DisplayInfo();

private:
  void InitVulkan();

  void InitSwapchain();

  void InitDefaultRenderPass();

  void InitFramebuffers();

  void InitCommands();

  void InitSyncStructures();

  void InitPipelines();

  // load spir-v shader file
  void LoadShaderModule(const char* shaderPath, VkShaderModule* outShaderModule);

  void Draw();

  bool m_initialized{false};
  int m_frameNumber{0};
  SDL_Window* m_window{nullptr};
  VkExtent2D m_windowExtent{1600, 900};

  VkInstance m_instance;
  VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
  VkPhysicalDevice m_chosenGPU;
  VkDevice m_device;
  VkPhysicalDeviceProperties m_gpuProperties;

  VkSemaphore m_presentSemaphore;
  VkSemaphore m_renderSemaphore;
  VkFence m_renderFence;

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

  VkCommandPool m_commandPool;
  VkCommandBuffer m_commandBuffer;

  VkRenderPass m_renderPass;

  VkSurfaceKHR m_surface;
  VkSwapchainKHR m_swapchain;
  VkFormat m_swapchainImageFormat;

  std::vector<VkFramebuffer> m_framebuffers;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;

  VkPipelineLayout m_pipelineLayout;

  VkPipeline m_trianglePipeline;
  VkPipeline m_redTrianglePipeline;

  DestructionQueue m_mainDestructionQueue;

  PFN_vkDestroyDevice fp_vkDestroyDevice = nullptr;
  vkh::DispatchTable m_dispatchTable;
};
}  // namespace ege

#endif  //ENGINE_HPP
