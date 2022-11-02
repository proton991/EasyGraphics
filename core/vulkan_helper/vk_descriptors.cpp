#include "vk_descriptors.hpp"
#include <algorithm>
#include "vk_functions.hpp"

namespace vkh {
/** DescriptorAllocator **/
VkDescriptorPool DescriptorAllocator::CreatePool(const DescriptorAllocator::PoolSizes& poolSizes,
                                                 int count, VkDescriptorPoolCreateFlags flags) {
  std::vector<VkDescriptorPoolSize> sizes;
  sizes.reserve(poolSizes.sizes.size());
  for (auto sz : poolSizes.sizes) {
    sizes.push_back({sz.first, static_cast<uint32_t>(sz.second * count)});
  }
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags         = flags;
  poolInfo.maxSets       = count;
  poolInfo.poolSizeCount = sizes.size();
  poolInfo.pPoolSizes    = sizes.data();

  VkDescriptorPool descriptorPool;
  m_table.fp_vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool);

  return descriptorPool;
}
void DescriptorAllocator::ResetPools() {
  for (auto p : m_usedPools) {
    m_table.fp_vkResetDescriptorPool(m_device, p, 0);
  }

  m_freePools = m_usedPools;
  m_usedPools.clear();
  m_currentPool = VK_NULL_HANDLE;
}

bool DescriptorAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout) {
  if (m_currentPool == VK_NULL_HANDLE) {
    m_currentPool = GrabPool();
    m_usedPools.push_back(m_currentPool);
  }

  VkDescriptorSetAllocateInfo allocateInfo{};
  allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocateInfo.pNext              = nullptr;
  allocateInfo.pSetLayouts        = &layout;
  allocateInfo.descriptorPool     = m_currentPool;
  allocateInfo.descriptorSetCount = 1;

  VkResult allocResult = m_table.fp_vkAllocateDescriptorSets(m_device, &allocateInfo, set);
  bool needReAllocate  = false;
  switch (allocResult) {
    case VK_SUCCESS:
      return true;
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      needReAllocate = true;
      break;
    default:
      return false;
  }
  if (needReAllocate) {
    m_currentPool = GrabPool();
    m_usedPools.push_back(m_currentPool);
    // use the new pool for allocation
    allocateInfo.descriptorPool = m_currentPool;

    allocResult = m_table.fp_vkAllocateDescriptorSets(m_device, &allocateInfo, set);
    if (allocResult == VK_SUCCESS) {
      return true;
    }
  }
  return false;
}

void DescriptorAllocator::Init(VkDevice device) {
  this->m_device = device;

  m_table.fp_vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(
      VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr(m_device, "vkCreateDescriptorPool"));

  m_table.fp_vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(
      VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr(m_device, "vkResetDescriptorPool"));

  m_table.fp_vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(
      VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr(m_device, "vkDestroyDescriptorPool"));

  m_table.fp_vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(
      VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr(m_device, "vkAllocateDescriptorSets"));
}

void DescriptorAllocator::Cleanup() {
  for (auto p : m_freePools) {
    m_table.fp_vkDestroyDescriptorPool(m_device, p, nullptr);
  }
  for (auto p : m_usedPools) {
    m_table.fp_vkDestroyDescriptorPool(m_device, p, nullptr);
  }
}

VkDescriptorPool DescriptorAllocator::GrabPool() {
  if (!m_freePools.empty()) {
    VkDescriptorPool pool = m_freePools.back();
    m_freePools.pop_back();
    return pool;
  } else {
    return CreatePool(m_poolSizes, 1000, 0);
  }
}

/** DescriptorLayoutCache **/
void DescriptorLayoutCache::Init(VkDevice device) {
  m_device = device;

  m_table.fp_vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(
      VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr(m_device,
                                                           "vkDestroyDescriptorSetLayout"));

  m_table.fp_vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(
      VulkanFunction::GetInstance().fp_vkGetDeviceProcAddr(m_device,
                                                           "vkCreateDescriptorSetLayout"));
}

void DescriptorLayoutCache::Cleanup() {
  for (auto p : m_layoutCache) {
    m_table.fp_vkDestroyDescriptorSetLayout(m_device, p.second, nullptr);
  }
}

VkDescriptorSetLayout DescriptorLayoutCache::CreateDescriptorSetLayout(
    VkDescriptorSetLayoutCreateInfo* info) {
  DescriptorLayoutInfo layoutInfo;
  layoutInfo.bindings.reserve(info->bindingCount);
  bool isSorted   = true;
  int lastBinding = -1;

  for (int i = 0; i < info->bindingCount; i++) {
    layoutInfo.bindings.push_back(info->pBindings[i]);
    if (info->pBindings[i].binding > lastBinding) {
      lastBinding = info->pBindings[i].binding;
    } else {
      isSorted = false;
    }
  }

  if (!isSorted) {
    std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(),
              [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
                return a.binding < b.binding;
              });
  }

  auto it = m_layoutCache.find(layoutInfo);
  if (it != m_layoutCache.end()) {
    return (*it).second;
  } else {
    VkDescriptorSetLayout layout;
    m_table.fp_vkCreateDescriptorSetLayout(m_device, info, nullptr, &layout);

    m_layoutCache[layoutInfo] = layout;
    return layout;
  }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(
    const DescriptorLayoutCache::DescriptorLayoutInfo& other) const {
  if (other.bindings.size() != bindings.size()) {
    return false;
  } else {
    //compare each of the bindings is the same. Bindings are sorted so they will match
    for (int i = 0; i < bindings.size(); i++) {
      if (other.bindings[i].binding != bindings[i].binding) {
        return false;
      }
      if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
        return false;
      }
      if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
        return false;
      }
      if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
        return false;
      }
    }
    return true;
  }
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
  using std::hash;
  using std::size_t;

  size_t result = hash<size_t>()(bindings.size());

  for (const VkDescriptorSetLayoutBinding& b : bindings) {
    //pack the binding data into a single int64. Not fully correct but it's ok
    size_t binding_hash =
        b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

    //shuffle the packed binding data and xor it with the main hash
    result ^= hash<size_t>()(binding_hash);
  }

  return result;
}

/** DescriptorBuilder **/
DescriptorBuilder DescriptorBuilder::Begin(DescriptorLayoutCache* layoutCache,
                                           DescriptorAllocator* allocator,
                                           PFN_vkUpdateDescriptorSets fp_vkUpdateDescriptorSets) {
  DescriptorBuilder builder;
  builder.m_cache     = layoutCache;
  builder.m_allocator = allocator;

  builder.m_table.fp_vkUpdateDescriptorSets = fp_vkUpdateDescriptorSets;

  return builder;
}

DescriptorBuilder& DescriptorBuilder::BindBuffer(uint32_t binding,
                                                 VkDescriptorBufferInfo* bufferInfo,
                                                 VkDescriptorType type,
                                                 VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount    = 1;
  newBinding.descriptorType     = type;
  newBinding.pImmutableSamplers = nullptr;
  newBinding.stageFlags         = stageFlags;
  newBinding.binding            = binding;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext           = nullptr;
  newWrite.descriptorCount = 1;
  newWrite.descriptorType  = type;
  newWrite.pBufferInfo     = bufferInfo;
  newWrite.dstBinding      = binding;

  m_writes.push_back(newWrite);

  return *this;
}

DescriptorBuilder& DescriptorBuilder::BindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo,
                                                VkDescriptorType type,
                                                VkShaderStageFlags stageFlags) {
  VkDescriptorSetLayoutBinding newBinding{};
  newBinding.descriptorCount    = 1;
  newBinding.descriptorType     = type;
  newBinding.pImmutableSamplers = nullptr;
  newBinding.stageFlags         = stageFlags;
  newBinding.binding            = binding;

  m_bindings.push_back(newBinding);

  VkWriteDescriptorSet newWrite{};
  newWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  newWrite.pNext           = nullptr;
  newWrite.descriptorCount = 1;
  newWrite.descriptorType  = type;
  newWrite.pImageInfo      = imageInfo;
  newWrite.dstBinding      = binding;

  m_writes.push_back(newWrite);

  return *this;
}

bool DescriptorBuilder::Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout) {
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.pNext        = nullptr;
  layoutInfo.pBindings    = m_bindings.data();
  layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());

  layout = m_cache->CreateDescriptorSetLayout(&layoutInfo);

  if (!m_allocator->Allocate(&set, layout)) {
    return false;
  }

  for (VkWriteDescriptorSet& w : m_writes) {
    w.dstSet = set;
  }

  m_table.fp_vkUpdateDescriptorSets(m_allocator->m_device, static_cast<uint32_t>(m_writes.size()),
                                    m_writes.data(), 0, nullptr);

  return true;
}

bool DescriptorBuilder::Build(VkDescriptorSet& set) {
  VkDescriptorSetLayout layout;
  return Build(set, layout);
}
}  // namespace vkh