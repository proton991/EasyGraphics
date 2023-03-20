#include "device.hpp"
#include "logging.hpp"

namespace ezg::vulkan {
static const char* QUEUE_NAMES[] = {"Graphics", "Compute", "transfer", "video_decode"};
Device::~Device() {
  if (m_allocator) {
    vmaDestroyAllocator(m_allocator);
  }
}

void Device::SetContext(Context& ctx) {
  m_gpu        = ctx.m_gpu;
  m_instance   = std::move(ctx.m_instance);
  m_device     = std::move(ctx.m_device);
  m_queue_info = ctx.m_q_info;
  m_features   = ctx.m_features;
  m_gpu_props  = m_gpu.getProperties();
  m_mem_props  = m_gpu.getMemoryProperties();

  initVMA();
  displayInfo();
}

VmaAllocator Device::Allocator() const {
  return m_allocator;
}

void Device::initVMA() {
  VmaAllocatorCreateInfo allocatorInfo{};
  allocatorInfo.physicalDevice = m_gpu;
  allocatorInfo.device         = *m_device;
  allocatorInfo.instance       = *m_instance;
  // let VMA fetch vulkan function pointers dynamically
  VmaVulkanFunctions vmaVulkanFunctions{};
  vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vmaVulkanFunctions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

  allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
  if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
    logger::critical("Failed to initialize vma");
  }
}

void Device::displayInfo() {
  logger::info("Using GPU: {}", m_gpu_props.deviceName);
  logger::info("The GPU has a minimum buffer alignment of {}",
               m_gpu_props.limits.minUniformBufferOffsetAlignment);
  logger::info("Max number of color attachments: {}", m_gpu_props.limits.maxColorAttachments);
  logger::info("Using Device Queues: ");
  logger::SetListPattern();
  for (int i = 0; i < QUEUE_INDEX_COUNT; i++) {
    if (m_queue_info.family_indices[i] != VK_QUEUE_FAMILY_IGNORED) {
      logger::info("Queue: {}, family index: {}, queue index: {}", QUEUE_NAMES[i],
                   m_queue_info.family_indices[i], m_queue_info.queue_indices[i]);
    }
  }
  logger::SetDefaultPattern();
}
}  // namespace ezg::vulkan