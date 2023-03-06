#ifndef EASYGRAPHICS_CONTEXT_HPP
#define EASYGRAPHICS_CONTEXT_HPP
struct GLFWwindow;
namespace ezg::gl {

class Context {
public:
  explicit Context(GLFWwindow* window_handle);

  Context(Context&&)                 = delete;
  Context(const Context&)            = delete;
  Context& operator=(Context&&)      = delete;
  Context& operator=(const Context&) = delete;

private:
  void init_debug();
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_CONTEXT_HPP
