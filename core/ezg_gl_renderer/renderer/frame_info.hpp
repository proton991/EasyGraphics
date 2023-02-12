#ifndef FRAME_INFO_HPP
#define FRAME_INFO_HPP

namespace ezg::gl {
class ShaderProgram;
class Model;
struct FrameInfo {
  ShaderProgram& shader_program;
  Model& model;
};
}
#endif  //FRAME_INFO_HPP
