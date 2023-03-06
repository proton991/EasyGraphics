#include "render_target.hpp"

#include "log.hpp"
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

void RenderTarget::attach_color_texture(GLuint id, GLint internal_format, GLenum format,
                                        int index) const {
  if (m_multi_sampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_info.samples, internal_format,
                            m_info.width, m_info.height, GL_FALSE);
  } else {
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(id, 1, internal_format, m_info.width, m_info.height);
    glTextureSubImage2D(m_id, 0, 0, 0, m_info.width, m_info.height, format, GL_UNSIGNED_BYTE,
                        nullptr);
  }
  glNamedFramebufferTexture(m_id, index, id, 0);
}

void RenderTarget::attach_depth_texture(uint32_t id, GLint format) const {
  if (m_multi_sampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_info.samples, format, m_info.width,
                            m_info.height, GL_FALSE);
  } else {
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(id, 1, format, m_info.width, m_info.height);
  }
  glNamedFramebufferTexture(m_id, GL_DEPTH_STENCIL_ATTACHMENT, id, 0);
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
  std::vector<GLenum> buffers;
  if (!m_info.color_attachment_infos.empty()) {
    const auto num_color_attachments = m_info.color_attachment_infos.size();
    m_color_attachments.resize(num_color_attachments);
    buffers.reserve(num_color_attachments);
    glCreateTextures(m_texture_target, m_color_attachments.size(), m_color_attachments.data());
    for (size_t i = 0; i < num_color_attachments; i++) {
      m_name2index.try_emplace(m_info.color_attachment_infos[i].name, i);
      attach_color_texture(m_color_attachments[i], GL_SRGB8_ALPHA8, GL_RGBA, GL_COLOR_ATTACHMENT0 + i);
      buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
  }

  if (m_info.has_depth) {
    glCreateTextures(m_texture_target, 1, &m_depth_attachment);
    attach_depth_texture(m_depth_attachment, GL_DEPTH24_STENCIL8);
  }

  if (m_color_attachments.empty()) {
    glDrawBuffer(GL_NONE);
  } else {
    glDrawBuffers(buffers.size(), buffers.data());
  }
  auto status = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    spdlog::error("Framebuffer is not complete, status {}", status);
  }
}

void RenderTarget::bind_texture(std::string_view name) {
  const auto slot = m_name2index[name];
  glBindTextureUnit(slot, m_color_attachments[slot]);
}
}  // namespace ezg::gl