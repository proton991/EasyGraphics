#include "framebuffer.hpp"
#include "log.hpp"

namespace ezg::gl {
AttachmentInfo AttachmentInfo::Color(std::string name_, AttachmentBinding binding_, int w, int h) {
  AttachmentInfo info{};
  info.type    = AttachmentType::TEXTURE_2D;
  info.name    = std::move(name_);
  info.binding = binding_;
  info.width   = w;
  info.height  = h;
  return info;
}

AttachmentInfo AttachmentInfo::Depth(int w, int h) {
  AttachmentInfo info{};
  info.type            = AttachmentType::TEXTURE_2D;
  info.name            = "depth";
  info.binding         = AttachmentBinding::DEPTH;
  info.width           = w;
  info.height          = h;
  info.min_filter      = GL_NEAREST;
  info.mag_filter      = GL_NEAREST;
  info.wrap            = GL_CLAMP_TO_BORDER;
  info.internal_format = GL_DEPTH_COMPONENT32F;
  return info;
}

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
    if (info.generate_mipmap) {
      glGenerateTextureMipmap(m_id);
    }
  } else if (m_type == AttachmentType::TEXTURE_2D_MS) {
    // default 4xMSAA
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, info.internal_format, info.width,
                            info.height, GL_FALSE);
  } else if (m_type == AttachmentType::TEXTURE_CUBEMAP) {

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_id);
    glTextureStorage2D(m_id,
                       info.level,            //level: number of  mipmaps
                       info.internal_format,  // internal format
                       info.width, info.height);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, info.wrap);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, info.min_filter);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, info.mag_filter);
    if (info.generate_mipmap) {
      glGenerateTextureMipmap(m_id);
    }
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
    if (attachment->get_type() != AttachmentType::TEXTURE_CUBEMAP) {
      // cubemap texture need to be configured via attach_layer_texture
      glNamedFramebufferTexture(m_id, static_cast<int>(attachment->get_binding()),
                                attachment->get_id(), 0);
    }
    m_attachment_ids.push_back(attachment->get_id());
    if (attachment->get_binding() != AttachmentBinding::DEPTH &&
        attachment->get_binding() != AttachmentBinding::DEPTH_STENCIL) {
      buffers.push_back(static_cast<GLenum>(attachment->get_binding()));
    } else {
      m_has_depth_texture = true;
    }
    m_attachments.try_emplace(attachment->get_name(), attachment);
  }
  if (buffers.empty()) {
    glNamedFramebufferDrawBuffer(m_id, GL_NONE);
  } else {
    glNamedFramebufferDrawBuffers(m_id, buffers.size(), buffers.data());
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
  if (!m_has_depth_texture) {
    setup_depth_rbo();
  }
  auto status = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    spdlog::error("Framebuffer is not complete, status {}", status);
  }
  unbind();
}

void Framebuffer::attach_layer_texture(int layer, const std::string& name, int level) {
  const auto& attachment = m_attachments.at(name);
  glNamedFramebufferTextureLayer(m_id, static_cast<int>(attachment->get_binding()),
                                 attachment->get_id(), level, layer);
  glClearNamedFramebufferfv(m_id, GL_COLOR, 0, ClearColor);
  glClearNamedFramebufferfv(m_id, GL_DEPTH, 0, &ClearDepth);
}

void Framebuffer::resize_attachment(const std::string& name, int width, int height) {
  const auto& att = m_attachments.at(name);
  if (att->get_type() != AttachmentType::TEXTURE_CUBEMAP) {
    glTextureStorage2D(att->get_id(), 1, att->get_internal_format(), width, height);
    glNamedFramebufferTexture(m_id, static_cast<int>(att->get_binding()), att->get_id(), 0);
  }
}

void Framebuffer::resize_depth_renderbuffer(int width, int height) {
  glNamedRenderbufferStorage(m_depth_rbo, GL_DEPTH24_STENCIL8, width, height);
  glNamedFramebufferRenderbuffer(m_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_rbo);
}

void Framebuffer::add_attachment(const AttachmentInfo& attachment_info) {
  auto attachment = Attachment::Create(attachment_info);
  glNamedFramebufferTexture(m_id, static_cast<int>(attachment->get_binding()), attachment->get_id(),
                            0);
  m_attachment_ids.push_back(attachment->get_id());
  m_attachments.try_emplace(attachment->get_name(), attachment);
}

void Framebuffer::unbind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind_for_writing(bool set_view_port) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  if (set_view_port) {
    glViewport(0, 0, m_width, m_height);
  }
}

void Framebuffer::bind_for_reading(const std::string& name, int slot) const {
  const auto& attachment = m_attachments.at(name);
  glBindTextureUnit(slot, attachment->get_id());
}

void Framebuffer::clear() {
  glClearNamedFramebufferfv(m_id, GL_COLOR, 0, ClearColor);
  glClearNamedFramebufferfv(m_id, GL_DEPTH, 0, &ClearDepth);
}

void Framebuffer::clear_depth() {
  glClearNamedFramebufferfv(m_id, GL_DEPTH, 0, &ClearDepth);
}
}  // namespace ezg::gl