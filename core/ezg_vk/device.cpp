#include "device.hpp"

namespace ezg::vk {

void Device::SetContext(const Context& ctx) {
  m_instance    = ctx.m_instance;
  m_device      = ctx.m_device;
  m_gpu         = ctx.m_gpu;
  m_gpuFeats    = ctx.ext;
  m_gpuProps    = ctx.m_gpuProps;
  m_gpuMemProps = ctx.m_gpuMemProps;
  m_customQInfo = ctx.m_customQInfo;
}

void Device::DisplayInfo() {
  spd::info("Using GPU: {}", m_gpuProps.deviceName);
  spd::info("The GPU has a minimum buffer alignment of {}",
            m_gpuProps.limits.minUniformBufferOffsetAlignment);
  for (int i = 0; i < QUEUE_INDEX_COUNT; i++) {
    if (m_customQInfo.familyIndices[i] != VK_QUEUE_FAMILY_IGNORED) {
      spd::info("Queue: {}, family index: {}, queue index: {}", QUEUE_NAMES[i],
                m_customQInfo.familyIndices[i], m_customQInfo.qIndices[i]);
    }
  }
}

}  // namespace ezg::vk