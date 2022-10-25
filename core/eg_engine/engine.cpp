#include "engine.hpp"
#include <SDL2/SDL_vulkan.h>
#include <fstream>
#include <string>
#include "vulkan_helper/core.hpp"
#include "vulkan_helper/vk_init.hpp"
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>
#include <glm/gtx/transform.hpp>

namespace ege {
void EGEngine::Init() {
  // We initialize SDL and create a window with it.
  SDL_Init(SDL_INIT_VIDEO);

  auto windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  m_window = SDL_CreateWindow("EGEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              m_windowExtent.width, m_windowExtent.height, windowFlags);
  InitVulkan();

  InitVMA();

  InitSwapchain();

  InitDefaultRenderPass();

  InitFramebuffers();

  InitCommands();

  InitSyncStructures();

  InitPipelines();

  LoadMeshes();

  m_initialized = true;
}
void EGEngine::InitVulkan() {
  // Build instance
  vkh::Instance vkhInstance = vkh::InstanceBuilder()
                                  .EnableValidationLayers(true)
                                  .SetAppName("First App")
                                  .SetApiVersion(1, 1, 0)
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

void EGEngine::InitVMA() {
  VmaAllocatorCreateInfo allocatorInfo{};
  allocatorInfo.physicalDevice = m_chosenGPU;
  allocatorInfo.device         = m_device;
  allocatorInfo.instance       = m_instance;
  // let VMA fetch vulkan function pointers dynamically
  VmaVulkanFunctions vmaVulkanFunctions{};
  vmaVulkanFunctions.vkGetInstanceProcAddr =
      vkh::VulkanFunction::GetInstance().fp_vkGetInstanceProcAddr;
  vmaVulkanFunctions.vkGetDeviceProcAddr =
      vkh::VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr;
  allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
  vkh::VkCheck(vmaCreateAllocator(&allocatorInfo, &m_allocator), "initialize vma");
}

void EGEngine::InitSwapchain() {
  vkh::SwapchainBuilder swapchainBuilder{m_chosenGPU, m_device, m_surface,
                                         m_queueFamilyIndices.graphics,
                                         m_queueFamilyIndices.present};
  vkh::Swapchain vkhSwapchain = swapchainBuilder.UseDefaultFormat()
                                    .SetDesiredPresentMode(VK_PRESENT_MODE_FIFO_KHR)
                                    .SetExtent(m_windowExtent.width, m_windowExtent.height)
                                    .Build();
  m_swapchain            = vkhSwapchain.swapchain;
  m_swapchainImageFormat = vkhSwapchain.imageFormat;
  m_swapchainImages      = vkhSwapchain.GetImages();
  m_swapchainImageViews  = vkhSwapchain.GetImageViews();

  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.fp_vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  });
  VkExtent3D depthImageExtent      = {m_windowExtent.width, m_windowExtent.height, 1};
  VkImageCreateInfo depthImageInfo = vkh::init::ImageCreateInfo(
      m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

  VmaAllocationCreateInfo depthImageAllocInfo{};
  depthImageAllocInfo.usage         = VMA_MEMORY_USAGE_GPU_ONLY;
  depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vmaCreateImage(m_allocator, &depthImageInfo, &depthImageAllocInfo, &m_depthImage.m_image,
                 &m_depthImage.m_allocation, nullptr);

  VkImageViewCreateInfo depthImgViewInfo = vkh::init::ImageViewCreateInfo(
      m_depthFormat, m_depthImage.m_image, VK_IMAGE_ASPECT_DEPTH_BIT);

  vkh::VkCheck(
      m_dispatchTable.fp_vkCreateImageView(m_device, &depthImgViewInfo, nullptr, &m_depthImageView),
      "create depth image view");

  m_mainDestructionQueue.PushFunction([=] {
    m_dispatchTable.fp_vkDestroyImageView(m_device, m_depthImageView, nullptr);
    vmaDestroyImage(m_allocator, m_depthImage.m_image, m_depthImage.m_allocation);
  });
}

void EGEngine::InitDefaultRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format         = m_swapchainImageFormat;
  colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentReference{};
  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment = {};
  // Depth attachment
  depthAttachment.flags          = 0;
  depthAttachment.format         = m_depthFormat;
  depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef = {};
  depthAttachmentRef.attachment            = 1;
  depthAttachmentRef.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount    = 1;
  subpass.pColorAttachments       = &colorAttachmentReference;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};

  //1 dependency, which is from "outside" into the subpass. And we can read or write color
  VkSubpassDependency dependency{};
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkSubpassDependency depthDependency = {};
  depthDependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  depthDependency.dstSubpass          = 0;
  depthDependency.srcStageMask =
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  depthDependency.srcAccessMask = 0;
  depthDependency.dstStageMask =
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkSubpassDependency dependencies[2] = {dependency, depthDependency};

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 2;
  renderPassInfo.pAttachments    = &attachments[0];
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 2;
  renderPassInfo.pDependencies   = &dependencies[0];

  vkh::VkCheck(
      m_dispatchTable.fp_vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass),
      "Create render pass");

  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.fp_vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  });
}

void EGEngine::InitFramebuffers() {
  VkFramebufferCreateInfo fbInfo = vkh::init::FramebufferCreateInfo(m_renderPass, m_windowExtent);

  const uint32_t imageCount = m_swapchainImages.size();
  m_framebuffers            = std::vector<VkFramebuffer>(imageCount);

  for (auto i = 0; i < imageCount; ++i) {
    VkImageView attachments[2];
    attachments[0] = m_swapchainImageViews[i];
    attachments[1] = m_depthImageView;

    fbInfo.attachmentCount = 2;
    fbInfo.pAttachments    = &attachments[0];

    vkh::VkCheck(
        m_dispatchTable.fp_vkCreateFramebuffer(m_device, &fbInfo, nullptr, &m_framebuffers[i]),
        "Create frame buffer");
    m_mainDestructionQueue.PushFunction([=] {
      m_dispatchTable.fp_vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
      m_dispatchTable.fp_vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
    });
  }
}

void EGEngine::InitCommands() {
  VkCommandPoolCreateInfo cmdPoolInfo = vkh::init::CommandPoolCreateInfo(
      m_queueFamilyIndices.graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  vkh::VkCheck(m_dispatchTable.fp_vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool),
               "Create command pool");

  VkCommandBufferAllocateInfo cmdBufferInfo =
      vkh::init::CommandBufferAllocateInfo(m_cmdPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  vkh::VkCheck(m_dispatchTable.fp_vkAllocateCommandBuffers(m_device, &cmdBufferInfo, &m_cmdBuffer),
               "Allocate command buffer");

  m_mainDestructionQueue.PushFunction([=] {
    m_dispatchTable.fp_vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
  });
}

void EGEngine::InitSyncStructures() {
  VkFenceCreateInfo fenceInfo = vkh::init::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  vkh::VkCheck(m_dispatchTable.fp_vkCreateFence(m_device, &fenceInfo, nullptr, &m_renderFence),
               "Create render fence");

  m_mainDestructionQueue.PushFunction([=] {
    m_dispatchTable.fp_vkDestroyFence(m_device, m_renderFence, nullptr);
  });
  VkSemaphoreCreateInfo semaphoreInfo = vkh::init::SemaphoreCreateInfo(0);
  vkh::VkCheck(
      m_dispatchTable.fp_vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_presentSemaphore),
      "Create present semaphore");
  vkh::VkCheck(
      m_dispatchTable.fp_vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderSemaphore),
      "Create render semaphore");

  m_mainDestructionQueue.PushFunction([=] {
    m_dispatchTable.fp_vkDestroySemaphore(m_device, m_presentSemaphore, nullptr);
    m_dispatchTable.fp_vkDestroySemaphore(m_device, m_renderSemaphore, nullptr);
  });
}

void EGEngine::InitPipelines() {

  VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkh::init::PipelineLayoutCreateInfo();
  VkPushConstantRange pushConstant;
  pushConstant.offset     = 0;
  pushConstant.size       = sizeof(MeshPushConstants);
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  meshPipelineLayoutInfo.pPushConstantRanges    = &pushConstant;
  meshPipelineLayoutInfo.pushConstantRangeCount = 1;

  vkh::VkCheck(m_dispatchTable.fp_vkCreatePipelineLayout(m_device, &meshPipelineLayoutInfo, nullptr,
                                                         &m_meshPipelineLayout),
               "create mesh pipeline layout");

  vkh::PipelineBuilder pipelineBuilder{m_dispatchTable.fp_vkCreateGraphicsPipelines};
  // read vertex data from vertex buffers
  pipelineBuilder.m_vertexInputInfo = vkh::init::VertexInputStateCreateInfo();
  pipelineBuilder.m_inputAssembly =
      vkh::init::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  //build viewport and scissor from the swapchain extents
  pipelineBuilder.m_viewport.x        = 0.0f;
  pipelineBuilder.m_viewport.y        = 0.0f;
  pipelineBuilder.m_viewport.width    = (float)m_windowExtent.width;
  pipelineBuilder.m_viewport.height   = (float)m_windowExtent.height;
  pipelineBuilder.m_viewport.minDepth = 0.0f;
  pipelineBuilder.m_viewport.maxDepth = 1.0f;
  pipelineBuilder.m_scissor.offset    = {0, 0};
  pipelineBuilder.m_scissor.extent    = m_windowExtent;

  pipelineBuilder.m_depthStencil =
      vkh::init::DepthStencilStateCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

  pipelineBuilder.m_rasterizer = vkh::init::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

  pipelineBuilder.m_multisampling = vkh::init::MultisampleStateCreateInfo();

  pipelineBuilder.m_colorBlendAttachment = vkh::init::ColorBlendAttachmentState();

  pipelineBuilder.m_pipelineLayout = m_meshPipelineLayout;

  VertexInputDescription vertexInputDescription = Vertex::GetVertexDescription();
  pipelineBuilder.m_vertexInputInfo.pVertexAttributeDescriptions =
      vertexInputDescription.attributes.data();
  pipelineBuilder.m_vertexInputInfo.vertexAttributeDescriptionCount =
      vertexInputDescription.attributes.size();
  pipelineBuilder.m_vertexInputInfo.pVertexBindingDescriptions =
      vertexInputDescription.bindings.data();
  pipelineBuilder.m_vertexInputInfo.vertexBindingDescriptionCount =
      vertexInputDescription.bindings.size();

  VkShaderModule meshVertShader;
  if (!LoadShaderModule("../shaders/spv/tri_mesh_pushconstants.vert.spv", &meshVertShader)) {
    vkh::Log("Error when building tri_mesh vertex shader module!");
  } else {
    vkh::Log("tri_mesh vertex shader successfully loaded!");
  }

  VkShaderModule triangleFragShader;
  if (!LoadShaderModule("../shaders/spv/colored_triangle.frag.spv", &triangleFragShader)) {
    vkh::Log("Error when building triangle fragment shader module!");
  } else {
    vkh::Log("Triangle fragment shader successfully loaded!");
  }

  pipelineBuilder.m_shaderStages.push_back(
      vkh::init::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

  pipelineBuilder.m_shaderStages.push_back(
      vkh::init::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));

  m_meshPipeline = pipelineBuilder.Build(m_device, m_renderPass);

  // delete shaders
  m_dispatchTable.fp_vkDestroyShaderModule(m_device, meshVertShader, nullptr);
  m_dispatchTable.fp_vkDestroyShaderModule(m_device, triangleFragShader, nullptr);

  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.fp_vkDestroyPipelineLayout(m_device, m_meshPipelineLayout, nullptr);
    m_dispatchTable.fp_vkDestroyPipeline(m_device, m_meshPipeline, nullptr);
  });
}

bool EGEngine::LoadShaderModule(const char* shaderPath, VkShaderModule* outShaderModule) {
  std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    return false;
  }
  size_t fileSize = static_cast<size_t>(file.tellg());
  //spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
  std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
  file.seekg(0);
  file.read((char*)buffer.data(), fileSize);
  file.close();

  VkShaderModuleCreateInfo shaderModuleInfo = {};

  shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleInfo.pNext = nullptr;
  // codeSize has to be in bytes
  shaderModuleInfo.codeSize = buffer.size() * sizeof(uint32_t);
  shaderModuleInfo.pCode    = buffer.data();
  VkShaderModule shaderModule;
  vkh::VkCheck(
      m_dispatchTable.fp_vkCreateShaderModule(m_device, &shaderModuleInfo, nullptr, &shaderModule),
      "Create shader module");
  *outShaderModule = shaderModule;
  return true;
}

void EGEngine::LoadMeshes() {
  //make the array 3 vertices long
  m_triangleMesh.m_vertices.resize(3);

  //vertex positions
  m_triangleMesh.m_vertices[0].position = {1.f, 1.f, 0.0f};
  m_triangleMesh.m_vertices[1].position = {-1.f, 1.f, 0.0f};
  m_triangleMesh.m_vertices[2].position = {0.f, -1.f, 0.0f};

  //vertex colors, all green
  m_triangleMesh.m_vertices[0].color = {0.f, 1.f, 0.0f};  //pure green
  m_triangleMesh.m_vertices[1].color = {0.f, 1.f, 0.0f};  //pure green
  m_triangleMesh.m_vertices[2].color = {0.f, 1.f, 0.0f};  //pure green

  //we don't care about the vertex normals

  UploadMesh(m_triangleMesh);
//  m_monkeyMesh.LoadFromObj("../assets/monkey_smooth.obj");
  m_monkeyMesh.LoadFromObj("../assets/smooth_vase.obj");
  UploadMesh(m_monkeyMesh);
}

void EGEngine::UploadMesh(Mesh& mesh) {
  // allocate vertex buffer
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = nullptr;
  // size in bytes
  bufferInfo.size  = mesh.m_vertices.size() * sizeof(Vertex);
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocationInfo{};
  vmaAllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  vkh::VkCheck(
      vmaCreateBuffer(m_allocator, &bufferInfo, &vmaAllocationInfo, &mesh.m_vertexBuffer.m_buffer,
                      &mesh.m_vertexBuffer.m_allocation, nullptr),
      "vma create buffer");

  m_mainDestructionQueue.PushFunction([=]() {
    vmaDestroyBuffer(m_allocator, mesh.m_vertexBuffer.m_buffer, mesh.m_vertexBuffer.m_allocation);
  });

  void* data;
  // copy vertex data
  vmaMapMemory(m_allocator, mesh.m_vertexBuffer.m_allocation, &data);

  memcpy(data, mesh.m_vertices.data(), mesh.m_vertices.size() * sizeof(Vertex));

  vmaUnmapMemory(m_allocator, mesh.m_vertexBuffer.m_allocation);
}

void EGEngine::Draw() {
  if (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) {
    return;
  }
  auto timeOut = std::numeric_limits<uint64_t>::max();
  vkh::VkCheck(m_dispatchTable.fp_vkWaitForFences(m_device, 1, &m_renderFence, true, timeOut),
               "Wait for fences");
  vkh::VkCheck(m_dispatchTable.fp_vkResetFences(m_device, 1, &m_renderFence), "Reset fences");
  vkh::VkCheck(m_dispatchTable.fp_vkResetCommandBuffer(m_cmdBuffer, 0), "Reset command buffer");

  uint32_t swapchainImageIndex;
  vkh::VkCheck(
      m_dispatchTable.fp_vkAcquireNextImageKHR(m_device, m_swapchain, timeOut, m_presentSemaphore,
                                               nullptr, &swapchainImageIndex),
      "Acquire image index");

  VkCommandBufferBeginInfo cmdBeginInfo =
      vkh::init::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkh::VkCheck(m_dispatchTable.fp_vkBeginCommandBuffer(m_cmdBuffer, &cmdBeginInfo),
               "begin command buffer");

  VkClearValue clearValue;
  float flash      = abs(sin(m_frameNumber / 120.f));
  clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  VkClearValue depthClear;
  depthClear.depthStencil.depth = 1.f;

  VkClearValue clearValues[2] = {clearValue, depthClear};

  VkRenderPassBeginInfo rpInfo = vkh::init::RenderpassBeginInfo(
      m_renderPass, m_windowExtent, m_framebuffers[swapchainImageIndex]);

  rpInfo.clearValueCount = 2;
  rpInfo.pClearValues    = &clearValues[0];

  m_dispatchTable.fp_vkCmdBeginRenderPass(m_cmdBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

  m_dispatchTable.fp_vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                       m_meshPipeline);

  VkDeviceSize offset = 0;
  m_dispatchTable.fp_vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1,
                                            &m_monkeyMesh.m_vertexBuffer.m_buffer, &offset);

  glm::vec3 camPos     = {0.f, 0.f, -2.f};
  glm::mat4 view       = glm::translate(glm::mat4(1.f), camPos);
  float aspect         = 4.0f / 3.0f;
  glm::mat4 projection = glm::perspective(glm::radians(90.f), aspect, 0.1f, 200.0f);
  projection[1][1] *= -1;
  glm::mat4 model =
      glm::rotate(glm::mat4{1.0f}, glm::radians(m_frameNumber * 0.4f), glm::vec3(0, 1, 0));
//    glm::mat4 meshMatrix = projection * view * model;
  model = glm::scale(model, {5, 5, 5});
  model = glm::translate(model, {0, 0, 0});
  glm::mat4 meshMatrix = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix() * model;

  MeshPushConstants constants;
  constants.renderMatrix = meshMatrix;
  m_dispatchTable.fp_vkCmdPushConstants(m_cmdBuffer, m_meshPipelineLayout,
                                        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                                        &constants);

  //  m_dispatchTable.fp_vkCmdDraw(m_cmdBuffer, m_triangleMesh.m_vertices.size(), 1, 0, 0);
  m_dispatchTable.fp_vkCmdDraw(m_cmdBuffer, m_monkeyMesh.m_vertices.size(), 1, 0, 0);
  //  m_dispatchTable.fp_vkCmdDraw(m_cmdBuffer, 3, 1, 0, 0);

  m_dispatchTable.fp_vkCmdEndRenderPass(m_cmdBuffer);

  vkh::VkCheck(m_dispatchTable.fp_vkEndCommandBuffer(m_cmdBuffer), "end command buffer");

  VkSubmitInfo submitInfo        = vkh::init::SubmitInfo(&m_cmdBuffer);
  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  submitInfo.pWaitDstStageMask = &waitStage;

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = &m_presentSemaphore;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &m_renderSemaphore;

  //submit command buffer to the queue and execute it.
  vkh::VkCheck(
      m_dispatchTable.fp_vkQueueSubmit(m_queueFamilies.graphics, 1, &submitInfo, m_renderFence),
      "submit to graphics queue");

  VkPresentInfoKHR presentInfo = vkh::init::PresentInfo();

  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains    = &m_swapchain;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = &m_renderSemaphore;

  presentInfo.pImageIndices = &swapchainImageIndex;

  vkh::VkCheck(m_dispatchTable.fp_vkQueuePresentKHR(m_queueFamilies.graphics, &presentInfo),
               "queue present");

  m_frameNumber++;
}

void EGEngine::Run() {
  bool bQuit = false;
  auto currentTime = std::chrono::high_resolution_clock::now();
  //main loop
  while (!bQuit) {
    SDL_Event e;
    //Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
      currentTime = newTime;
      m_camera.Update(&e, frameTime);
      //close the window when user alt-f4s or clicks the X button
      if (e.type == SDL_QUIT) {
        bQuit = true;
      }
    }
    Draw();
  }
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
  std::cout << "Present Queue: " << m_queueFamilyIndices.present << "\n";
  vkh::Log("\n=========================================");
}

void EGEngine::Destroy() {
  if (m_initialized) {
    m_dispatchTable.fp_vkDeviceWaitIdle(m_device);

    m_mainDestructionQueue.Flush();

    vmaDestroyAllocator(m_allocator);

    vkh::VulkanFunction::GetInstance().fp_vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    fp_vkDestroyDevice(m_device, nullptr);

    vkh::DestroyDebugUtilsMessenger(m_instance, m_debugUtilsMessenger, nullptr);

    vkh::VulkanFunction::GetInstance().fp_vkDestroyInstance(m_instance, nullptr);

    SDL_DestroyWindow(m_window);
  }
}
}  // namespace ege
