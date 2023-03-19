#ifndef COMMON_HPP
#define COMMON_HPP
#include <memory>
namespace ezg::vulkan {
template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
}  // namespace ezg::vulkan
#endif  //COMMON_HPP
