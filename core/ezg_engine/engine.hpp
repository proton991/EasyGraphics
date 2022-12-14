#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <SDL2/SDL.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <deque>
#include <functional>
#include <memory>

#include "camera.hpp"
#include "material_system.hpp"
#include "mesh.hpp"
#include "scene_system.hpp"
#include "vulkan_helper/vk_descriptors.hpp"
#include "vulkan_helper/vk_device.hpp"
#include "vulkan_helper/vk_dispatch.hpp"
#include "vulkan_helper/vk_pipeline.hpp"

#define MAX_OBJECTS 10000
namespace ezg {
//number of frames to overlap when rendering
constexpr unsigned int FRAME_OVERLAP = 2;

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
  friend class SceneSystem;

  void Init();

  void Run();

  void Destroy();

  void DisplayInfo();

  //  const uint64_t TIME_OUT = std::numeric_limits<uint64_t>::max();
  const uint64_t TIME_OUT = 1000000000;

private:
  void InitVulkan();

  void InitVMA();

  void InitSwapchain();

  void InitDefaultRenderPass();

  void InitFramebuffers();

  void InitCommands();

  void InitSyncStructures();

  void InitDescriptors();

  void InitPipelines();

  void InitScene();

  // load spir-v shader file
  bool LoadShaderModule(const char* shaderPath, VkShaderModule* outShaderModule);

  void LoadMeshes();

  void LoadImages();

  void UploadMesh(Mesh& mesh);

  void RenderScene();

  void Draw();

  size_t PadUniformBufferSize(size_t originalSize);

  AllocatedBuffer CreateBuffer(size_t bufferSize, VkBufferUsageFlags usage,
                               VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaFlags);

  void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

  FrameData& GetCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; }

  bool m_initialized{false};
  int m_frameNumber{0};
  SDL_Window* m_window{nullptr};
  VkExtent2D m_windowExtent{1600, 900};

  VkInstance m_instance;
  VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
  std::unique_ptr<vkh::debug::DebugUtil> m_debugUtil;
  VkPhysicalDevice m_chosenGPU;
  VkDevice m_device;
  VkPhysicalDeviceProperties m_gpuProperties;

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

  FrameData m_frames[FRAME_OVERLAP];

  VkRenderPass m_renderPass;

  VkSurfaceKHR m_surface;
  VkSwapchainKHR m_swapchain;
  VkFormat m_swapchainImageFormat;

  std::vector<VkFramebuffer> m_framebuffers;
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;

  DestructionQueue m_mainDestructionQueue;

  PFN_vkDestroyDevice fp_vkDestroyDevice = nullptr;
  vkh::DispatchTable m_dispatchTable;

  VmaAllocator m_allocator;

  VkImageView m_depthImageView;
  AllocatedImage m_depthImage;

  VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;

  MaterialSystem m_materialSystem;

  SceneSystem m_sceneSystem;

  Camera m_camera{glm::vec3(0.0f, -1.0f, -2.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f),
                  90.0f,
                  4.0f / 3.0f,
                  0.1f,
                  200.0f};

  std::unordered_map<std::string, Mesh> m_meshes;

  vkh::DescriptorAllocator* m_descriptorAllocator;
  vkh::DescriptorLayoutCache* m_descriptorLayoutCache;

  VkDescriptorSetLayout m_globalSetLayout;
  VkDescriptorSetLayout m_objectSetLayout;
  VkDescriptorSetLayout m_singleTextureSetLayout;

  GPUSceneData m_sceneParameters;
  AllocatedBuffer m_sceneParameterBuffer;

  UploadContext m_uploadContext;

  std::unordered_map<std::string, Texture> m_loadedTextures;

  friend bool LoadImageFromFile(EGEngine& engine, const char* file, AllocatedImage& outImage);
  friend AllocatedImage UploadImage(int texWidth, int texHeight, VkFormat image_format,
                                    EGEngine& engine, AllocatedBuffer& stagingBuffer);
};
}  // namespace ezg

#endif  //ENGINE_HPP
