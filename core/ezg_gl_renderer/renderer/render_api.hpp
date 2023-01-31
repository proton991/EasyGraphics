#ifndef EASYGRAPHICS_RENDER_API_HPP
#define EASYGRAPHICS_RENDER_API_HPP

#include <glm/glm.hpp>
#include <memory>

namespace ezg::gl {
class BaseVAO;

class RenderAPI {
public:
  static void set_clear_color(const glm::vec4& color);
  static void clear();

  static void draw_vertices(const std::shared_ptr<BaseVAO>& vao, uint32_t num_vertices);
  static void draw_indices(const std::shared_ptr<BaseVAO>& vao, uint32_t num_indices);
};

}
#endif  //EASYGRAPHICS_RENDER_API_HPP
