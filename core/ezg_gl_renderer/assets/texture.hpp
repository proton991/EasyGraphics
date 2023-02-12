#ifndef EASYGRAPHICS_TEXTURE_HPP
#define EASYGRAPHICS_TEXTURE_HPP
#include <glad/glad.h>
#include <string>
#include <memory>

namespace ezg::gl {
class Texture2D;
using TexturePtr = std::shared_ptr<Texture2D>;
struct TextureInfo {
  int width{0};
  int height{0};
  int min_filter{GL_LINEAR};
  int mag_filter{GL_LINEAR};
  int wrap_s{GL_REPEAT};
  int wrap_t{GL_REPEAT};
  bool generate_mipmap{false};
};
class Texture2D {
public:
  static TexturePtr create(const std::string& path) {
    return std::make_shared<Texture2D>(path);
  }

  static TexturePtr Create(const TextureInfo& info, const void* data) {
    return std::make_shared<Texture2D>(info, data);
  }

  static TexturePtr CreateDefaultWhite();

  explicit Texture2D(const std::string& path);
  Texture2D(const TextureInfo& info, const void* data);
  ~Texture2D();

  void bind(GLenum slot) const;

  uint32_t get_id() const { return m_id; }
private:
  uint32_t m_id{0};
  int m_width;
  int m_height;
  GLenum m_internal_format, m_data_format;
};
}
#endif  //EASYGRAPHICS_TEXTURE_HPP
