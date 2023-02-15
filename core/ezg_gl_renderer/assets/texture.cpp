#include <stb_image.h>
#include "texture.hpp"
#include <spdlog/spdlog.h>


namespace ezg::gl {
Texture2D::Texture2D(const std::string& path) {
  m_internal_format = GL_RGBA8;
  m_data_format     = GL_RGBA;

  int width, height, channels;
  stbi_set_flip_vertically_on_load(1);
  stbi_uc* data = nullptr;
  spdlog::trace("Loading texture at path {}", path);
  data = stbi_load(path.c_str(), &width, &height, &channels, 0);
  if (data) {
    m_width  = width;
    m_height = height;
    if (channels == 4) {
      m_internal_format = GL_RGBA8;
      m_data_format     = GL_RGBA;
    } else if (channels == 3) {
      m_internal_format = GL_RGB8;
      m_data_format     = GL_RGB;
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, m_internal_format, m_width, m_height);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, m_data_format, GL_UNSIGNED_BYTE, data);
//    glGenerateTextureMipmap(m_id);

    stbi_image_free(data);
  } else {
    spdlog::error("Failed to load texture {}", path);
  }
}

Texture2D::Texture2D(const TextureInfo& info, const void* data) {
  m_internal_format = GL_RGBA8;
  m_data_format     = GL_RGBA;
  m_width  = info.width;
  m_height = info.height;

  glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
  glTextureStorage2D(m_id, 1, m_internal_format, m_width, m_height);

  glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, info.wrap_s);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, info.wrap_t);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, info.wrap_r);
  glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, info.min_filter);
  glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, info.mag_filter);

  glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, m_data_format, GL_UNSIGNED_BYTE, data);
  if (info.generate_mipmap) {
    glGenerateTextureMipmap(m_id);
  }

}

TexturePtr Texture2D::CreateDefaultWhite() {
  TextureInfo info{};
  info.width = 1;
  info.height = 1;
  float white[] = {1, 1, 1, 1};
  return std::make_shared<Texture2D>(info, white);
}

Texture2D::~Texture2D() {
  if (m_id != 0) {
    glDeleteTextures(1, &m_id);
  }

}
void Texture2D::bind(GLenum slot) const {
  glBindTextureUnit(slot, m_id);
//  glBindTexture(GL_TEXTURE_2D, m_id);
}
}  // namespace ezg::gl