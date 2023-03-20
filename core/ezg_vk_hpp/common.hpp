#ifndef COMMON_HPP
#define COMMON_HPP
#include <memory>
#define EMPTY_QUEUE_FLAG vk::QueueFlags()
namespace ezg::vulkan {
template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

enum QueueIndices {
  QUEUE_INDEX_GRAPHICS,
  QUEUE_INDEX_COMPUTE,
  QUEUE_INDEX_TRANSFER,
  QUEUE_INDEX_VIDEO_DECODE,
  QUEUE_INDEX_COUNT
};

}  // namespace ezg::vulkan
#endif  //COMMON_HPP
