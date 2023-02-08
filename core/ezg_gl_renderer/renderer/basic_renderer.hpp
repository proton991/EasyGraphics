#ifndef EASYGRAPHICS_BASIC_RENDERER_HPP
#define EASYGRAPHICS_BASIC_RENDERER_HPP

#include <vector>
#include "assets/model.hpp"
#include "graphics/shader.hpp"
namespace ezg::gl {
class BasicRenderer {
public:
  void init();
  void destroy();

  void render_model(const ModelPtr& model, ShaderProgram& shader_program);
};
}
#endif  //EASYGRAPHICS_BASIC_RENDERER_HPP
