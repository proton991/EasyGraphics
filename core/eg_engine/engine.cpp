#include "engine.hpp"
#include <SDL2/SDL_vulkan.h>
#include <fstream>
#include <string>
#include "texture.hpp"
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

  InitDescriptors();

  InitPipelines();

  LoadImages();

  LoadMeshes();

  InitScene();

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
  VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features{};
  shader_draw_parameters_features.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
  shader_draw_parameters_features.pNext                = nullptr;
  shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

  vkh::Device vkhDevice = deviceBuilder.addPNext(&shader_draw_parameters_features).Build();

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
  std::cout << "The GPU has a minimum buffer alignment of "
            << m_gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;
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
    m_dispatchTable.destroySwapchainKHR(m_swapchain, nullptr);
  });
  VkExtent3D depthImageExtent      = {m_windowExtent.width, m_windowExtent.height, 1};
  VkImageCreateInfo depthImageInfo = vkh::init::ImageCreateInfo(
      m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

  VmaAllocationCreateInfo depthImageAllocInfo{};
  depthImageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  depthImageAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  //  depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vmaCreateImage(m_allocator, &depthImageInfo, &depthImageAllocInfo, &m_depthImage.m_image,
                 &m_depthImage.m_allocation, nullptr);

  VkImageViewCreateInfo depthImgViewInfo = vkh::init::ImageViewCreateInfo(
      m_depthFormat, m_depthImage.m_image, VK_IMAGE_ASPECT_DEPTH_BIT);

  vkh::VkCheck(m_dispatchTable.createImageView(&depthImgViewInfo, nullptr, &m_depthImageView),
               "create depth image view");

  m_mainDestructionQueue.PushFunction([=] {
    m_dispatchTable.destroyImageView(m_depthImageView, nullptr);
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

  vkh::VkCheck(m_dispatchTable.createRenderPass(&renderPassInfo, nullptr, &m_renderPass),
               "Create render pass");

  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.destroyRenderPass(m_renderPass, nullptr);
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

    vkh::VkCheck(m_dispatchTable.createFramebuffer(&fbInfo, nullptr, &m_framebuffers[i]),
                 "Create frame buffer");
    m_mainDestructionQueue.PushFunction([=] {
      m_dispatchTable.destroyFramebuffer(m_framebuffers[i], nullptr);
      m_dispatchTable.destroyImageView(m_swapchainImageViews[i], nullptr);
    });
  }
}

void EGEngine::InitCommands() {
  VkCommandPoolCreateInfo cmdPoolInfo = vkh::init::CommandPoolCreateInfo(
      m_queueFamilyIndices.graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  for (int i = 0; i < FRAME_OVERLAP; ++i) {
    vkh::VkCheck(m_dispatchTable.createCommandPool(&cmdPoolInfo, nullptr, &m_frames[i].cmdPool),
                 "Create command pool");

    VkCommandBufferAllocateInfo cmdBufferInfo = vkh::init::CommandBufferAllocateInfo(
        m_frames[i].cmdPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    vkh::VkCheck(m_dispatchTable.allocateCommandBuffers(&cmdBufferInfo, &m_frames[i].cmdBuffer),
                 "Allocate command buffer");

    m_mainDestructionQueue.PushFunction([=] {
      m_dispatchTable.destroyCommandPool(m_frames[i].cmdPool, nullptr);
    });
  }
  VkCommandPoolCreateInfo uploadCmdPoolInfo =
      vkh::init::CommandPoolCreateInfo(m_queueFamilyIndices.graphics);
  vkh::VkCheck(
      m_dispatchTable.createCommandPool(&uploadCmdPoolInfo, nullptr, &m_uploadContext.cmdPool),
      "create upload context command pool");
  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.destroyCommandPool(m_uploadContext.cmdPool, nullptr);
  });
}

void EGEngine::InitSyncStructures() {
  VkFenceCreateInfo fenceInfo = vkh::init::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

  VkSemaphoreCreateInfo semaphoreInfo = vkh::init::SemaphoreCreateInfo(0);
  for (int i = 0; i < FRAME_OVERLAP; ++i) {
    vkh::VkCheck(m_dispatchTable.createFence(&fenceInfo, nullptr, &m_frames[i].renderFence),
                 "Create render fence");
    m_mainDestructionQueue.PushFunction([=] {
      m_dispatchTable.destroyFence(m_frames[i].renderFence, nullptr);
    });
    vkh::VkCheck(
        m_dispatchTable.createSemaphore(&semaphoreInfo, nullptr, &m_frames[i].presentSemaphore),
        "Create present semaphore");
    vkh::VkCheck(
        m_dispatchTable.createSemaphore(&semaphoreInfo, nullptr, &m_frames[i].renderSemaphore),
        "Create render semaphore");
    m_mainDestructionQueue.PushFunction([=] {
      m_dispatchTable.destroySemaphore(m_frames[i].presentSemaphore, nullptr);
      m_dispatchTable.destroySemaphore(m_frames[i].renderSemaphore, nullptr);
    });
  }

  VkFenceCreateInfo uploadFenceInfo = vkh::init::FenceCreateInfo();
  vkh::VkCheck(m_dispatchTable.createFence(&uploadFenceInfo, nullptr, &m_uploadContext.uploadFence),
               "create upload context fence");
  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.destroyFence(m_uploadContext.uploadFence, nullptr);
  });
}

void EGEngine::InitDescriptors() {
  m_descriptorAllocator = new vkh::DescriptorAllocator();
  m_descriptorAllocator->Init(m_device);

  m_descriptorLayoutCache = new vkh::DescriptorLayoutCache();
  m_descriptorLayoutCache->Init(m_device);

  const size_t sceneParamBufferSize = FRAME_OVERLAP * PadUniformBufferSize(sizeof(GPUSceneData));
  m_sceneParameterBuffer =
      CreateBuffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO,
                   VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  VkDescriptorSetLayoutBinding textureBind = vkh::init::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
  VkDescriptorSetLayoutCreateInfo setInfo{};
  setInfo.bindingCount = 1;
  setInfo.flags        = 0;
  setInfo.pNext        = nullptr;
  setInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  setInfo.pBindings    = &textureBind;

  m_singleTextureSetLayout = m_descriptorLayoutCache->CreateDescriptorSetLayout(&setInfo);

  for (int i = 0; i < FRAME_OVERLAP; ++i) {
    m_frames[i].descriptorAllocator = new vkh::DescriptorAllocator();
    m_frames[i].descriptorAllocator->Init(m_device);

    m_frames[i].cameraBuffer =
        CreateBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    m_frames[i].objectBuffer =
        CreateBuffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                     VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    VkDescriptorBufferInfo cameraBufferInfo = m_frames[i].cameraBuffer.GetDescriptorBufferInfo();
    VkDescriptorBufferInfo sceneBufferInfo  = m_sceneParameterBuffer.GetDescriptorBufferInfo();

    sceneBufferInfo.range = sizeof(GPUSceneData);

    VkDescriptorBufferInfo objBufferInfo = m_frames[i].objectBuffer.GetDescriptorBufferInfo();

    vkh::DescriptorBuilder::Begin(m_descriptorLayoutCache, m_frames[i].descriptorAllocator)
        .BindBuffer(0, &cameraBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_VERTEX_BIT)
        .BindBuffer(1, &sceneBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(m_frames[i].globalDescriptor, m_globalSetLayout);

    vkh::DescriptorBuilder::Begin(m_descriptorLayoutCache, m_frames[i].descriptorAllocator)
        .BindBuffer(0, &objBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_VERTEX_BIT)
        .Build(m_frames[i].objectDescriptor, m_objectSetLayout);
  }
  m_mainDestructionQueue.PushFunction([&]() {
    vmaDestroyBuffer(m_allocator, m_sceneParameterBuffer.m_buffer,
                     m_sceneParameterBuffer.m_allocation);
    for (int i = 0; i < FRAME_OVERLAP; ++i) {
      m_frames[i].descriptorAllocator->Cleanup();
      vmaDestroyBuffer(m_allocator, m_frames[i].cameraBuffer.m_buffer,
                       m_frames[i].cameraBuffer.m_allocation);
      vmaDestroyBuffer(m_allocator, m_frames[i].objectBuffer.m_buffer,
                       m_frames[i].objectBuffer.m_allocation);
    }
  });
}

void EGEngine::InitPipelines() {

  VkShaderModule meshVertShader;
  if (!LoadShaderModule("../shaders/spv/tri_mesh_ssbo.vert.spv", &meshVertShader)) {
    vkh::Log("Error when building tri_mesh vertex shader module!");
  } else {
    vkh::Log("tri_mesh_ssbo vertex shader successfully loaded!");
  }

  VkShaderModule colorFragShader;
  if (!LoadShaderModule("../shaders/spv/default_lit.frag.spv", &colorFragShader)) {
    vkh::Log("Error when building default_lit fragment shader module!");
  } else {
    vkh::Log("default_lit fragment shader successfully loaded!");
  }

  VkShaderModule texturedFragShader;
  if (!LoadShaderModule("../shaders/spv/textured_lit.frag.spv", &texturedFragShader)) {
    vkh::Log("Error when building textured_lit fragment shader ");
  } else {
    vkh::Log("textured_lit fragment shader successfully loaded!");
  }

  VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkh::init::PipelineLayoutCreateInfo();
  VkPushConstantRange pushConstant;
  pushConstant.offset     = 0;
  pushConstant.size       = sizeof(MeshPushConstants);
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  meshPipelineLayoutInfo.pPushConstantRanges    = &pushConstant;
  meshPipelineLayoutInfo.pushConstantRangeCount = 1;

  VkDescriptorSetLayout setLayouts[] = {m_globalSetLayout, m_objectSetLayout};

  meshPipelineLayoutInfo.setLayoutCount = 2;
  meshPipelineLayoutInfo.pSetLayouts    = setLayouts;

  VkPipelineLayout meshPipelineLayout;
  VkPipeline meshPipeline;
  vkh::VkCheck(
      m_dispatchTable.createPipelineLayout(&meshPipelineLayoutInfo, nullptr, &meshPipelineLayout),
      "create mesh pipeline layout");

  VkPipelineLayoutCreateInfo texturedPipelineLayoutInfo = meshPipelineLayoutInfo;

  VkDescriptorSetLayout texturedSetLayouts[] = {m_globalSetLayout, m_objectSetLayout,
                                                m_singleTextureSetLayout};

  texturedPipelineLayoutInfo.setLayoutCount = 3;
  texturedPipelineLayoutInfo.pSetLayouts    = texturedSetLayouts;

  VkPipelineLayout texturedPipeLayout;
  vkh::VkCheck(m_dispatchTable.createPipelineLayout(&texturedPipelineLayoutInfo, nullptr,
                                                    &texturedPipeLayout),
               "create texture pipeline layout");

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

  pipelineBuilder.m_pipelineLayout = meshPipelineLayout;

  VertexInputDescription vertexInputDescription = Vertex::GetVertexDescription();
  pipelineBuilder.m_vertexInputInfo.pVertexAttributeDescriptions =
      vertexInputDescription.attributes.data();
  pipelineBuilder.m_vertexInputInfo.vertexAttributeDescriptionCount =
      vertexInputDescription.attributes.size();
  pipelineBuilder.m_vertexInputInfo.pVertexBindingDescriptions =
      vertexInputDescription.bindings.data();
  pipelineBuilder.m_vertexInputInfo.vertexBindingDescriptionCount =
      vertexInputDescription.bindings.size();

  pipelineBuilder.m_shaderStages.push_back(
      vkh::init::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

  pipelineBuilder.m_shaderStages.push_back(
      vkh::init::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colorFragShader));

  meshPipeline = pipelineBuilder.Build(m_device, m_renderPass);

  pipelineBuilder.m_shaderStages.clear();
  pipelineBuilder.m_shaderStages.push_back(
      vkh::init::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

  pipelineBuilder.m_shaderStages.push_back(
      vkh::init::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, texturedFragShader));
  pipelineBuilder.m_pipelineLayout = texturedPipeLayout;
  VkPipeline texturePipeline       = pipelineBuilder.Build(m_device, m_renderPass);
  m_materialSystem.CreateMaterial("textured", texturePipeline, texturedPipeLayout);
  // delete shaders
  m_dispatchTable.destroyShaderModule(meshVertShader, nullptr);
  m_dispatchTable.destroyShaderModule(colorFragShader, nullptr);
  m_dispatchTable.destroyShaderModule(texturedFragShader, nullptr);

  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.destroyPipelineLayout(meshPipelineLayout, nullptr);
    m_dispatchTable.destroyPipelineLayout(texturedPipeLayout, nullptr);
    m_dispatchTable.destroyPipeline(meshPipeline, nullptr);
    m_dispatchTable.destroyPipeline(texturePipeline, nullptr);
  });

  m_materialSystem.CreateMaterial("default", meshPipeline, meshPipelineLayout);
}

void EGEngine::InitScene() {
  glm::mat4 vaseMat = glm::scale(glm::mat4{1.0f}, {3.f, 3.f, 3.f});
  vaseMat           = glm::translate(vaseMat, {-0.5f, 0, 0});
  std::vector<SceneObjectInfo> sceneObjInfos;
  SceneObjectInfo smoothVaseObjInfo{};
  smoothVaseObjInfo.material        = m_materialSystem.GetMaterial("textured");
  smoothVaseObjInfo.mesh            = &m_meshes["smoothVase"];
  smoothVaseObjInfo.transformMatrix = vaseMat;
  sceneObjInfos.push_back(smoothVaseObjInfo);

  glm::mat4 vaseMat2 = glm::scale(glm::mat4{1.0f}, {3.f, 3.f, 3.f});
  vaseMat2           = glm::translate(vaseMat2, {0.5f, 0, 0});
  SceneObjectInfo flatVaseObjInfo{};
  flatVaseObjInfo.material        = m_materialSystem.GetMaterial("textured");
  flatVaseObjInfo.mesh            = &m_meshes["flatVase"];
  flatVaseObjInfo.transformMatrix = vaseMat2;
  sceneObjInfos.push_back(flatVaseObjInfo);

  for (int x = -20; x <= 20; x++) {
    for (int y = -20; y <= 20; y++) {
      SceneObjectInfo tri;
      tri.mesh              = &m_meshes["tri"];
      tri.material          = m_materialSystem.GetMaterial("default");
      glm::mat4 translation = glm::translate(glm::mat4{1.0}, glm::vec3(x, 0.0f, y));
      glm::mat4 scale       = glm::scale(glm::mat4{1.0}, glm::vec3(0.2, 0.2, 0.2));
      tri.transformMatrix   = translation * scale;
      sceneObjInfos.push_back(tri);
    }
  }
  m_sceneSystem.AddObjectBatch(sceneObjInfos.data(), sceneObjInfos.size());

  Material* texturedMat = m_materialSystem.GetMaterial("textured");

  VkSamplerCreateInfo sampleInfo = vkh::init::SamplerCreateInfo(VK_FILTER_NEAREST);
  VkSampler basicSampler;
  m_dispatchTable.createSampler(&sampleInfo, nullptr, &basicSampler);
  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.destroySampler(basicSampler, nullptr);
  });
  VkDescriptorImageInfo imageBufferInfo;
  imageBufferInfo.sampler     = basicSampler;
  imageBufferInfo.imageView   = m_loadedTextures["board"].imageView;
  imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  vkh::DescriptorBuilder::Begin(m_descriptorLayoutCache, m_descriptorAllocator)
      .BindImage(0, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                 VK_SHADER_STAGE_FRAGMENT_BIT)
      .Build(texturedMat->textureSet, m_singleTextureSetLayout);
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
  vkh::VkCheck(m_dispatchTable.createShaderModule(&shaderModuleInfo, nullptr, &shaderModule),
               "Create shader module");
  *outShaderModule = shaderModule;
  return true;
}

void EGEngine::LoadMeshes() {
  Mesh triMesh{};
  //make the array 3 vertices long
  triMesh.m_vertices.resize(3);

  //vertex positions
  triMesh.m_vertices[0].position = {1.f, 1.f, 0.0f};
  triMesh.m_vertices[1].position = {-1.f, 1.f, 0.0f};
  triMesh.m_vertices[2].position = {0.f, -1.f, 0.0f};

  //vertex colors, all green
  triMesh.m_vertices[0].color = {0.f, 1.f, 0.0f};  //pure green
  triMesh.m_vertices[1].color = {0.f, 1.f, 0.0f};  //pure green
  triMesh.m_vertices[2].color = {0.f, 1.f, 0.0f};  //pure green

  Mesh smoothVaseMesh{};
  smoothVaseMesh.LoadFromObj("../assets/smooth_vase.obj");

  Mesh flatVaseMesh{};
  flatVaseMesh.LoadFromObj("../assets/flat_vase.obj");

  UploadMesh(triMesh);
  UploadMesh(smoothVaseMesh);
  UploadMesh(flatVaseMesh);

  m_meshes["tri"]        = triMesh;
  m_meshes["smoothVase"] = smoothVaseMesh;
  m_meshes["flatVase"]   = flatVaseMesh;
}

void EGEngine::LoadImages() {
  Texture board;
  LoadImageFromFile(*this, "../assets/missing.png", board.image);

  VkImageViewCreateInfo imageViewInfo = vkh::init::ImageViewCreateInfo(
      VK_FORMAT_R8G8B8A8_SRGB, board.image.m_image, VK_IMAGE_ASPECT_COLOR_BIT);

  m_dispatchTable.createImageView(&imageViewInfo, nullptr, &board.imageView);

  m_mainDestructionQueue.PushFunction([=]() {
    m_dispatchTable.destroyImageView(board.imageView, nullptr);
  });

  m_loadedTextures["board"] = board;
}

void EGEngine::UploadMesh(Mesh& mesh) {
  const size_t bufferSize = mesh.m_vertices.size() * sizeof(Vertex);

  //  VkBufferCreateInfo stagingBufferInfo{};
  //  stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //  stagingBufferInfo.pNext = nullptr;
  //  stagingBufferInfo.size  = bufferSize;
  //  stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  //
  //  VmaAllocationCreateInfo vmaAllocationInfo{};
  //  vmaAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
  //  vmaAllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

  //  AllocatedBuffer stagingBuffer;
  AllocatedBuffer stagingBuffer =
      CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                   VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
  //  vkh::VkCheck(vmaCreateBuffer(m_allocator, &stagingBufferInfo, &vmaAllocationInfo,
  //                               &stagingBuffer.m_buffer, &stagingBuffer.m_allocation, nullptr),
  //               "vma create staging buffer");

  //copy vertex data
  void* data;
  vmaMapMemory(m_allocator, stagingBuffer.m_allocation, &data);

  memcpy(data, mesh.m_vertices.data(), bufferSize);

  vmaUnmapMemory(m_allocator, stagingBuffer.m_allocation);
  //  // allocate vertex buffer
  //  VkBufferCreateInfo vertexBufferInfo{};
  //  vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //  vertexBufferInfo.pNext = nullptr;
  //  // size in bytes
  //  vertexBufferInfo.size  = bufferSize;
  //  vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  //  // let the VMA library know that this data should be gpu native
  //  vmaAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  // allocate vertex buffer
  mesh.m_vertexBuffer =
      CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                   VMA_MEMORY_USAGE_AUTO, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
  //  vkh::VkCheck(
  //      vmaCreateBuffer(m_allocator, &vertexBufferInfo, &vmaAllocationInfo,
  //                      &mesh.m_vertexBuffer.m_buffer, &mesh.m_vertexBuffer.m_allocation, nullptr),
  //      "vma create buffer");

  m_mainDestructionQueue.PushFunction([=]() {
    vmaDestroyBuffer(m_allocator, mesh.m_vertexBuffer.m_buffer, mesh.m_vertexBuffer.m_allocation);
  });

  ImmediateSubmit([=](VkCommandBuffer cmd) {
    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size      = bufferSize;
    m_dispatchTable.cmdCopyBuffer(cmd, stagingBuffer.m_buffer, mesh.m_vertexBuffer.m_buffer, 1,
                                  &copy);
  });
  vmaDestroyBuffer(m_allocator, stagingBuffer.m_buffer, stagingBuffer.m_allocation);
}

void EGEngine::Draw() {
  if (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) {
    return;
  }
  vkh::VkCheck(m_dispatchTable.waitForFences(1, &GetCurrentFrame().renderFence, true, TIME_OUT),
               "Wait for fences");
  vkh::VkCheck(m_dispatchTable.resetFences(1, &GetCurrentFrame().renderFence), "Reset fences");
  vkh::VkCheck(m_dispatchTable.resetCommandBuffer(GetCurrentFrame().cmdBuffer, 0),
               "Reset command buffer");

  uint32_t swapchainImageIndex;
  vkh::VkCheck(
      m_dispatchTable.acquireNextImageKHR(m_swapchain, TIME_OUT, GetCurrentFrame().presentSemaphore,
                                          nullptr, &swapchainImageIndex),
      "Acquire image index");

  VkCommandBufferBeginInfo cmdBeginInfo =
      vkh::init::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkh::VkCheck(m_dispatchTable.beginCommandBuffer(GetCurrentFrame().cmdBuffer, &cmdBeginInfo),
               "begin command buffer");

  VkClearValue clearValue;
  clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  VkClearValue depthClear;
  depthClear.depthStencil.depth = 1.f;

  VkClearValue clearValues[2] = {clearValue, depthClear};

  VkRenderPassBeginInfo rpInfo = vkh::init::RenderpassBeginInfo(
      m_renderPass, m_windowExtent, m_framebuffers[swapchainImageIndex]);

  rpInfo.clearValueCount = 2;
  rpInfo.pClearValues    = &clearValues[0];

  m_dispatchTable.cmdBeginRenderPass(GetCurrentFrame().cmdBuffer, &rpInfo,
                                     VK_SUBPASS_CONTENTS_INLINE);

  RenderScene();

  m_dispatchTable.cmdEndRenderPass(GetCurrentFrame().cmdBuffer);

  vkh::VkCheck(m_dispatchTable.endCommandBuffer(GetCurrentFrame().cmdBuffer), "end command buffer");

  VkSubmitInfo submitInfo        = vkh::init::SubmitInfo(&GetCurrentFrame().cmdBuffer);
  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  submitInfo.pWaitDstStageMask = &waitStage;

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = &GetCurrentFrame().presentSemaphore;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &GetCurrentFrame().renderSemaphore;

  //submit command buffer to the queue and execute it.
  vkh::VkCheck(m_dispatchTable.queueSubmit(m_queueFamilies.graphics, 1, &submitInfo,
                                           GetCurrentFrame().renderFence),
               "submit to graphics queue");

  VkPresentInfoKHR presentInfo = vkh::init::PresentInfo();

  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains    = &m_swapchain;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = &GetCurrentFrame().renderSemaphore;

  presentInfo.pImageIndices = &swapchainImageIndex;

  vkh::VkCheck(m_dispatchTable.queuePresentKHR(m_queueFamilies.graphics, &presentInfo),
               "queue present");

  m_frameNumber++;
}

void EGEngine::Run() {
  bool bQuit       = false;
  auto currentTime = std::chrono::high_resolution_clock::now();
  // place cursor at the center of the screen
  SDL_WarpMouseInWindow(m_window, m_windowExtent.width / 2, m_windowExtent.height / 2);
  SDL_ShowCursor(SDL_DISABLE);
  //main loop
  while (!bQuit) {
    SDL_Event e;
    //Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      //close the window when user alt-f4s or clicks the X button
      if (e.type == SDL_QUIT) {
        bQuit = true;
      } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        bQuit = true;
      }
    }
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;
    m_camera.Update(frameTime);
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
    m_dispatchTable.deviceWaitIdle();

    m_mainDestructionQueue.Flush();

    vmaDestroyAllocator(m_allocator);

    m_descriptorAllocator->Cleanup();
    m_descriptorLayoutCache->Cleanup();

    vkh::VulkanFunction::GetInstance().fp_vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    fp_vkDestroyDevice(m_device, nullptr);

    vkh::DestroyDebugUtilsMessenger(m_instance, m_debugUtilsMessenger, nullptr);

    vkh::VulkanFunction::GetInstance().fp_vkDestroyInstance(m_instance, nullptr);

    SDL_DestroyWindow(m_window);
  }
}

AllocatedBuffer EGEngine::CreateBuffer(size_t bufferSize, VkBufferUsageFlags usage,
                                       VmaMemoryUsage memoryUsage,
                                       VmaAllocationCreateFlags vmaFlags) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = nullptr;
  bufferInfo.size  = bufferSize;
  bufferInfo.usage = usage;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = memoryUsage;
  vmaAllocInfo.flags = vmaFlags;

  AllocatedBuffer buffer{};
  vkh::VkCheck(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaAllocInfo, &buffer.m_buffer,
                               &buffer.m_allocation, nullptr),
               "create buffer using vma");
  buffer.m_size = bufferSize;

  return buffer;
}

size_t EGEngine::PadUniformBufferSize(size_t originalSize) {
  // Calculate required alignment based on minimum device offset alignment
  size_t minUboAlignment = m_gpuProperties.limits.minUniformBufferOffsetAlignment;
  size_t alignedSize     = originalSize;
  if (minUboAlignment > 0) {
    alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
  }
  return alignedSize;
}

void EGEngine::ImmediateSubmit(std::function<void(VkCommandBuffer cmdBuffer)>&& function) {
  VkCommandBuffer cmdBuffer;
  VkCommandBufferAllocateInfo cmdBufferAllocInfo =
      vkh::init::CommandBufferAllocateInfo(m_uploadContext.cmdPool, 1);
  vkh::VkCheck(m_dispatchTable.allocateCommandBuffers(&cmdBufferAllocInfo, &cmdBuffer),
               "allocate immediate submit cmd buffer");

  VkCommandBufferBeginInfo cmdBeginInfo =
      vkh::init::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  vkh::VkCheck(m_dispatchTable.beginCommandBuffer(cmdBuffer, &cmdBeginInfo),
               "begin one time submit command buffer");
  function(cmdBuffer);
  vkh::VkCheck(m_dispatchTable.endCommandBuffer(cmdBuffer), "end one time submit command buffer");

  VkSubmitInfo submitInfo = vkh::init::SubmitInfo(&cmdBuffer);
  vkh::VkCheck(m_dispatchTable.queueSubmit(m_queueFamilies.graphics, 1, &submitInfo,
                                           m_uploadContext.uploadFence),
               "submit command buffer");

  m_dispatchTable.waitForFences(1, &m_uploadContext.uploadFence, true, TIME_OUT);
  m_dispatchTable.resetFences(1, &m_uploadContext.uploadFence);

  m_dispatchTable.resetCommandPool(m_uploadContext.cmdPool, 0);
}

}  // namespace ege
