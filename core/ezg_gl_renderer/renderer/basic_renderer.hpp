#ifndef EASYGRAPHICS_BASIC_RENDERER_HPP
#define EASYGRAPHICS_BASIC_RENDERER_HPP

#include <vector>
#include "assets/model.hpp"
namespace ezg::gl {
class BasicRenderer {
public:
  void init();
  void destroy();

  void render_models(const std::vector<ModelPtr> models);
};
}
#endif  //EASYGRAPHICS_BASIC_RENDERER_HPP
