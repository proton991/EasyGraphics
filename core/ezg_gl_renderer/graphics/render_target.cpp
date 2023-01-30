#include "render_target.hpp"

#include <spdlog/spdlog.h>
#include <utility>

namespace ezg::gl {

static GLenum texture_target(bool multi_sampled) {
  return multi_sampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static void create_textures(GLuint* ids, int count, bool multi_sampled) {
  glCreateTextures(texture_target(GL_TEXTURE_2D), count, ids);
}

static void bind_texture(GLuint id) {
  glBindTexture(texture_target(GL_TEXTURE_2D), id);
}

static void attach_color_texture(GLuint id, int samples, GLint internal_format, GLenum format,
                                 int width, int height, int index) {
  bool multi_sampled = samples > 1;
  if (multi_sampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height,
                            GL_FALSE);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE,
                 nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, index, texture_target(multi_sampled), id, 0);
}

static void attach_depth_texture(uint32_t id, int samples, GLint format, int width, int height) {
  bool multi_sampled = samples > 1;
  if (multi_sampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
  } else {
    glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_target(multi_sampled), id, 0);
}

RenderTarget::RenderTarget(RenderTargetInfo rt_info) : m_info(std::move(rt_info)) {
  invalidate();
}

RenderTarget::~RenderTarget() {
  glDeleteFramebuffers(1, &m_id);
  glDeleteTextures(m_color_attachments.size(), m_color_attachments.data());
  glDeleteTextures(1, &m_depth_attachment);
}

void RenderTarget::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  glViewport(0, 0, m_info.width, m_info.height);
}

void RenderTarget::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::resize(int width, int height) {
  m_info.width  = width;
  m_info.height = height;
  spdlog::trace("Resized to {} x {}", width, height);
  invalidate();
}

void RenderTarget::invalidate() {
  if (m_id != 0) {
    glDeleteFramebuffers(1, &m_id);
    glDeleteTextures(1, m_color_attachments.data());
    glDeleteTextures(1, &m_depth_attachment);
    m_color_attachments.clear();
    m_depth_attachment = 0;
  }

  glCreateFramebuffers(1, &m_id);
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  bool multi_sample = m_info.samples > 1;
  std::vector<GLenum> buffers;
  buffers.reserve(m_color_attachments.size());
  if (!m_info.color_attachment_infos.empty()) {
    m_color_attachments.resize(m_info.color_attachment_infos.size());
    create_textures(m_color_attachments.data(), m_color_attachments.size(), multi_sample);
    for (size_t i = 0; i < m_color_attachments.size(); i++) {
      bind_texture(m_color_attachments[i]);
      attach_color_texture(m_color_attachments[i], m_info.samples, GL_RGBA8, GL_RGBA, m_info.width,
                           m_info.height, GL_COLOR_ATTACHMENT0 + i);
      buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
  }

  if (m_info.has_depth) {
    create_textures(&m_depth_attachment, 1, multi_sample);
    bind_texture(m_depth_attachment);
    attach_depth_texture(m_depth_attachment, m_info.samples, GL_DEPTH24_STENCIL8, m_info.width,
                         m_info.height);
  }

  if (m_color_attachments.empty()) {
    glDrawBuffer(GL_NONE);
  } else {
    glDrawBuffers(buffers.size(), buffers.data());
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace ezg::gl