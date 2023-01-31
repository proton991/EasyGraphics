#ifndef EASYGRAPHICS_TEXTURE_HPP
#define EASYGRAPHICS_TEXTURE_HPP
#include <glad/glad.h>
#include <string>
#include <memory>

namespace ezg::gl {
class Texture2D;
using TexturePtr = std::shared_ptr<Texture2D>;

class Texture2D {
public:
  static TexturePtr create(const std::string& path) {
    return std::make_shared<Texture2D>(path);
  }

  explicit Texture2D(const std::string& path);
  ~Texture2D();

  void bind(GLenum slot) const;

private:
  uint32_t m_id{0};
  int m_width;
  int m_height;
  GLenum m_internal_format, m_data_format;
};
}
#endif  //EASYGRAPHICS_TEXTURE_HPP
