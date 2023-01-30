#ifndef EASYGRAPHICS_BASIC_RENDERER_HPP
#define EASYGRAPHICS_BASIC_RENDERER_HPP

namespace ezg::gl {

class BasicRenderer {
public:
  void init();
  void destroy();

  void renderModels();
};
}
#endif  //EASYGRAPHICS_BASIC_RENDERER_HPP
