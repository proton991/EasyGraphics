#ifndef EASYGRAPHICS_SHADER_MANAGER_HPP
#define EASYGRAPHICS_SHADER_MANAGER_HPP

#include <unordered_map>
#include "graphics/shader.hpp"

namespace ezg::gl {

class ShaderManager {
public:


private:
  std::unordered_map<std::string, ShaderProgram> m_shader_cache;
};
}
#endif  //EASYGRAPHICS_SHADER_MANAGER_HPP
