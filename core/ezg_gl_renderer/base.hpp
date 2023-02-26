#ifndef BASE_HPP
#define BASE_HPP
#include <memory>
namespace ezg::gl {
template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
  return std::make_shared<T>(std::forward<Args>(args)...);
}
}
#endif  //BASE_HPP
