#ifndef EASYGRAPHICS_TEXTURE_HPP
#define EASYGRAPHICS_TEXTURE_HPP
#include <glad/glad.h>
#include <string>
#include "base.hpp"

namespace ezg::gl {
struct TextureInfo {
  int width{0};
  int height{0};
  int min_filter{GL_LINEAR};
  int mag_filter{GL_LINEAR};
  int wrap_s{GL_REPEAT};
  int wrap_t{GL_REPEAT};
  int wrap_r{GL_REPEAT};
  bool generate_mipmap{false};
  GLenum internal_format{GL_RGBA8};
  GLenum data_format{GL_RGBA};
  GLenum data_type{GL_UNSIGNED_BYTE};
};
class Texture2D {
public:
  static Ref<Texture2D> Create(const std::string& path) {
    return CreateRef<Texture2D>(path);
  }

  static Ref<Texture2D> Create(const TextureInfo& info, const void* data) {
    return CreateRef<Texture2D>(info, data);
  }

  static Ref<Texture2D> CreateDefaultWhite();

  explicit Texture2D(const std::string& path);
  Texture2D(const TextureInfo& info, const void* data);
  ~Texture2D();

  void bind(GLenum slot) const;

  uint32_t get_id() const { return m_id; }

  GLuint64 get_handle() const { return m_handle; }
private:
  uint32_t m_id{0};
  GLuint64 m_handle{0};
  int m_width;
  int m_height;
  GLenum m_internal_format, m_data_format;
};

class TextureCubeMap {
public:
  static Ref<TextureCubeMap> Create(const TextureInfo& info, std::array<unsigned char*, 6> face_data);
  TextureCubeMap(const TextureInfo& info, const std::array<unsigned char*, 6>& face_data);
  ~TextureCubeMap();
  void bind(GLenum slot) const;
private:
  uint32_t m_id{0};
};
}
#endif  //EASYGRAPHICS_TEXTURE_HPP
