#include "framebuffer.hpp"
#include <spdlog/spdlog.h>

namespace ezg::gl {

Ref<Attachment> Attachment::Create(const AttachmentInfo& info) {
  return CreateRef<Attachment>(info);
}

Attachment::Attachment(const AttachmentInfo& info)
    : m_type(info.type), m_binding(info.binding), m_name(info.name) {
  if (m_type == AttachmentType::TEXTURE_2D) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, info.min_filter);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, info.mag_filter);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, info.wrap);
    glTextureStorage2D(m_id, 1, info.internal_format, info.width, info.height);
  } else if (m_type == AttachmentType::TEXTURE_2D_MS) {
    // default 4xMSAA
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, info.internal_format, info.width,
                            info.height, GL_FALSE);
  } else if (m_type == AttachmentType::TEXTURE_CUBEMAP) {

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_id);
    glTextureStorage2D(m_id,
                       1,                     // one level, no mipmaps
                       info.internal_format,  // internal format
                       info.width, info.height);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, info.min_filter);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, info.mag_filter);
  }
}

void Framebuffer::setup_depth_rbo() {
  glCreateRenderbuffers(1, &m_depth_rbo);
  glNamedRenderbufferStorage(m_depth_rbo, GL_DEPTH24_STENCIL8, m_width, m_height);
  glNamedFramebufferRenderbuffer(m_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_rbo);
}

Ref<Framebuffer> Framebuffer::Create(const FramebufferCreatInfo& info) {
  return CreateRef<Framebuffer>(info);
}

Framebuffer::Framebuffer(const FramebufferCreatInfo& info)
    : m_width(info.width), m_height(info.height), m_attachments_infos(info.attachments_infos) {
  invalidate();
}

Framebuffer::~Framebuffer() {
  glDeleteFramebuffers(1, &m_id);
  glDeleteTextures(m_attachment_ids.size(), m_attachment_ids.data());
  m_attachments.clear();
  m_attachment_ids.clear();
}
void Framebuffer::setup_attachments() {
  std::vector<GLenum> buffers;
  for (auto& attachment_info : m_attachments_infos) {
    // create attachment
    auto attachment = Attachment::Create(attachment_info);
    if (attachment->get_type() == AttachmentType::TEXTURE_CUBEMAP) {
      for (int i = 0; i < 6; i++) {
        glNamedFramebufferTextureLayer(m_id, static_cast<int>(attachment->get_binding()),
                                       attachment->get_id(), 0, i);
        glClearNamedFramebufferfv(m_id, GL_COLOR, 0, ClearColor);
        glClearNamedFramebufferfv(m_id, GL_DEPTH, 0, &ClearDepth);
      }
    } else {
      glNamedFramebufferTexture(m_id, static_cast<int>(attachment->get_binding()),
                                attachment->get_id(), 0);
    }
    m_attachment_ids.push_back(attachment->get_id());
    buffers.push_back(static_cast<GLenum>(attachment->get_binding()));
    m_attachments.try_emplace(attachment->get_name(), attachment);
  }
  if (buffers.empty()) {
    glDrawBuffer(GL_NONE);
  } else {
    glDrawBuffers(buffers.size(), buffers.data());
  }
}

void Framebuffer::invalidate() {
  if (m_id != 0) {
    glDeleteFramebuffers(1, &m_id);
    glDeleteTextures(m_attachment_ids.size(), m_attachment_ids.data());
    m_attachments.clear();
    m_attachment_ids.clear();
  }
  glCreateFramebuffers(1, &m_id);
  setup_attachments();
  setup_depth_rbo();
  auto status = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    spdlog::error("Framebuffer is not complete, status {}", status);
  }
}

void Framebuffer::attach_layer_texture(int layer, const std::string& name) {
  const auto& attachment = m_attachments.at(name);
  glNamedFramebufferTextureLayer(m_id, static_cast<int>(attachment->get_binding()),
                                 attachment->get_id(), 0, layer);
  glClearNamedFramebufferfv(m_id, GL_COLOR, 0, ClearColor);
  glClearNamedFramebufferfv(m_id, GL_DEPTH, 0, &ClearDepth);
}

void Framebuffer::resize_attachment(const std::string& name, int width, int height) {
  const auto& att = m_attachments.at(name);
  glTextureStorage2D(att->get_id(), 1, att->get_internal_format(), width, height);
  if (att->get_type() != AttachmentType::TEXTURE_CUBEMAP) {
    glNamedFramebufferTexture(m_id, static_cast<int>(att->get_binding()), att->get_id(), 0);
  }
}

void Framebuffer::resize_depth_renderbuffer(const std::string& name, int width, int height) {
  glNamedRenderbufferStorage(m_depth_rbo, GL_DEPTH24_STENCIL8, width, height);
  glNamedFramebufferRenderbuffer(m_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_rbo);
}

void Framebuffer::unbind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int Framebuffer::get_slot(const Ref<Attachment>& attachment) const {
  return static_cast<int>(attachment->get_binding()) - GL_COLOR_ATTACHMENT0;
}

void Framebuffer::bind(bool set_view_port) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  if (set_view_port) {
    glViewport(0, 0, m_width, m_height);
  }
}

void Framebuffer::bind_texture(const std::string& name, int slot) const {
  const auto& attachment = m_attachments.at(name);
  glBindTextureUnit(slot, attachment->get_id());
}
}  // namespace ezg::gl