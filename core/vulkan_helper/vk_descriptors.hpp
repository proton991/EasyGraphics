#ifndef VK_DESCRIPTORS_HPP
#define VK_DESCRIPTORS_HPP
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>

namespace vkh {
class DescriptorAllocator {
public:
  struct PoolSizes {
    std::vector<std::pair<VkDescriptorType, float>> sizes = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f}};
  };

  void ResetPools();

  bool Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

  void Init(VkDevice device);

  void Cleanup();

  VkDevice m_device;
private:
  VkDescriptorPool CreatePool(const DescriptorAllocator::PoolSizes& poolSizes, int count,
                              VkDescriptorPoolCreateFlags flags);

  VkDescriptorPool GrabPool();



  VkDescriptorPool m_currentPool{VK_NULL_HANDLE};
  PoolSizes m_poolSizes;
  std::vector<VkDescriptorPool> m_usedPools;
  std::vector<VkDescriptorPool> m_freePools;

  struct {
    PFN_vkCreateDescriptorPool fp_vkCreateDescriptorPool;
    PFN_vkResetDescriptorPool fp_vkResetDescriptorPool;
    PFN_vkDestroyDescriptorPool fp_vkDestroyDescriptorPool;

    PFN_vkAllocateDescriptorSets fp_vkAllocateDescriptorSets;
  } m_table;
};

class DescriptorLayoutCache {
public:
  void Init(VkDevice device);

  void Cleanup();

  VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo* info);

  struct DescriptorLayoutInfo {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    bool operator==(const DescriptorLayoutInfo& other) const;

    size_t hash() const;
  };

private:
  VkDevice m_device;
  struct DescriptorLayoutHash {
    size_t operator()(const DescriptorLayoutInfo& k) const { return k.hash(); }
  };
  std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash>
      m_layoutCache;
  struct {
    PFN_vkCreateDescriptorSetLayout fp_vkCreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout fp_vkDestroyDescriptorSetLayout;
  } m_table;
};

class DescriptorBuilder {
public:
  static DescriptorBuilder Begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator,
                                 PFN_vkUpdateDescriptorSets fp_vkUpdateDescriptorSets);

  DescriptorBuilder& BindBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo,
                                VkDescriptorType type, VkShaderStageFlags stageFlags);

  DescriptorBuilder& BindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo,
                               VkDescriptorType type, VkShaderStageFlags stageFlags);

  bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);

  bool Build(VkDescriptorSet& set);

private:
  std::vector<VkWriteDescriptorSet> m_writes;
  std::vector<VkDescriptorSetLayoutBinding> m_bindings;

  DescriptorLayoutCache* m_cache;
  DescriptorAllocator* m_allocator;

  struct {
    PFN_vkUpdateDescriptorSets fp_vkUpdateDescriptorSets;
  } m_table;
};
}  // namespace vkh
#endif  //VK_DESCRIPTORS_HPP
