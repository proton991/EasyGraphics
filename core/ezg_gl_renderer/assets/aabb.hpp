#ifndef AABB_HPP
#define AABB_HPP
#include <glm/glm.hpp>
#include "base.hpp"

namespace ezg::gl {
struct Line;
struct AABB {
  AABB() = default;
  AABB(const glm::vec3& _min, const glm::vec3& _max);

  glm::vec3 get_center() const;

  // translate the center of the bbx to target_pos
  void translate(const glm::vec3& pos);
  void scale(float factor);

  void get_lines_data(Ref<Line>& data) const;

  glm::vec3 bbx_min;
  glm::vec3 bbx_max;
  glm::vec3 diag;
};
}
#endif  //AABB_HPP
