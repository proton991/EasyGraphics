#include "render_target.hpp"

#include <spdlog/spdlog.h>
#include <utility>

namespace ezg::gl {

RenderTargetPtr RenderTarget::Create(const RenderTargetInfo& info) {
  return std::make_shared<RenderTarget>(info);
}

RenderTarget::RenderTarget(const RenderTargetInfo& rt_info) : m_info(rt_info) {
  if (rt_info.samples > 1) {
    m_multi_sampled  = true;
    m_texture_target = GL_TEXTURE_2D_MULTISAMPLE;
  } else {
    m_multi_sampled  = false;
    m_texture_target = GL_TEXTURE_2D;
  }
  invalidate();
}

RenderTarget::~RenderTarget() {
  glDeleteFramebuffers(1, &m_id);
  glDeleteTextures(m_color_attachments.size(), m_color_attachments.data());
  glDeleteTextures(1, &m_depth_attachment);
}

void RenderTarget::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  for (size_t i = 0; i < m_color_attachments.size(); i++) {
    glBindTextureUnit(i, m_color_attachments[i]);
  }
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

void RenderTarget::bind_texture(GLuint id) {
  glBindTexture(m_texture_target, id);
}

void RenderTarget::create_textures(GLuint* ids, int count) {
  glCreateTextures(m_texture_target, count, ids);
}

void RenderTarget::attach_color_texture(GLuint id, GLint internal_format, GLenum format,
                                        int index) const {
  if (m_multi_sampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_info.samples, internal_format,
                            m_info.width, m_info.height, GL_FALSE);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_info.width, m_info.height, 0, format,
                 GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, index, m_texture_target, id, 0);
}

void RenderTarget::attach_depth_texture(uint32_t id, GLint format) const {
  if (m_multi_sampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_info.samples, format, m_info.width,
                            m_info.height, GL_FALSE);
  } else {
    glTexStorage2D(GL_TEXTURE_2D, 1, format, m_info.width, m_info.height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_texture_target, id, 0);
}

void RenderTarget::invalidate() {
  if (m_id != 0) {
    glDeleteFramebuffers(1, &m_id);
    glDeleteTextures(m_color_attachments.size(), m_color_attachments.data());
    glDeleteTextures(1, &m_depth_attachment);
    m_color_attachments.clear();
    m_depth_attachment = 0;
  }

  glCreateFramebuffers(1, &m_id);
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  bool multi_sample = m_info.samples > 1;
  std::vector<GLenum> buffers;
  if (!m_info.color_attachment_infos.empty()) {
    m_color_attachments.resize(m_info.color_attachment_infos.size());
    buffers.reserve(m_color_attachments.size());
    create_textures(m_color_attachments.data(), m_color_attachments.size());
    for (size_t i = 0; i < m_color_attachments.size(); i++) {
      bind_texture(m_color_attachments[i]);
      //      glBindTextureUnit(i, m_color_attachments[i]);
      attach_color_texture(m_color_attachments[i], GL_RGBA8, GL_RGBA, GL_COLOR_ATTACHMENT0 + i);
      buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
  }

  if (m_info.has_depth) {
    create_textures(&m_depth_attachment, 1);
    bind_texture(m_depth_attachment);
    attach_depth_texture(m_depth_attachment, GL_DEPTH24_STENCIL8);
  }

  if (m_color_attachments.empty()) {
    glDrawBuffer(GL_NONE);
  } else {
    glDrawBuffers(buffers.size(), buffers.data());
  }
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    spdlog::error("Framebuffer is not complete!");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace ezg::gl