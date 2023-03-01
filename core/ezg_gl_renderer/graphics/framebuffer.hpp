#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "base.hpp"
#include "graphics/uniform_buffer.hpp"
#include "graphics/vertex_array.hpp"

namespace ezg::gl {
enum class AttachmentType { TEXTURE_2D, TEXTURE_2D_MS, TEXTURE_CUBEMAP, RENDER_BUFFER };

enum class AttachmentBinding : decltype(GL_COLOR_ATTACHMENT0) {
  COLOR0 = GL_COLOR_ATTACHMENT0,
  COLOR1 = GL_COLOR_ATTACHMENT1,
  COLOR2 = GL_COLOR_ATTACHMENT2,
  COLOR3 = GL_COLOR_ATTACHMENT3,
  COLOR4 = GL_COLOR_ATTACHMENT4,

  DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
};

struct AttachmentInfo {
  static auto Color(std::string name_, AttachmentBinding binding_, int w, int h) {
    AttachmentInfo info{};
    info.type    = AttachmentType::TEXTURE_2D;
    info.name    = std::move(name_);
    info.binding = binding_;
    info.width   = w;
    info.height  = h;
    return info;
  }
  int width{0};
  int height{0};
  AttachmentType type;
  AttachmentBinding binding;
  std::string name;
  // having default value
  int wrap{GL_CLAMP_TO_EDGE};
  int mag_filter{GL_LINEAR};
  int min_filter{GL_LINEAR};
  GLenum internal_format{GL_RGBA8};
  GLenum data_format{GL_RGBA};
  GLenum data_type{GL_UNSIGNED_BYTE};
  bool generate_mipmap{false};
};

class Attachment {
public:
  static Ref<Attachment> Create(const AttachmentInfo& info);

  Attachment(const AttachmentInfo& info);

  [[nodiscard]] auto get_type() const { return m_type; }
  [[nodiscard]] auto get_id() const { return m_id; }
  [[nodiscard]] auto get_binding() const { return m_binding; }
  [[nodiscard]] const auto& get_name() const { return m_name; }
  [[nodiscard]] const auto& get_internal_format() const { return m_internal_format; }

private:
  uint32_t m_id{0};
  AttachmentType m_type;
  AttachmentBinding m_binding;
  std::string m_name;
  GLenum m_internal_format;
};
struct FramebufferCreatInfo {
  uint32_t width{800};
  uint32_t height{800};
  std::vector<AttachmentInfo> attachments_infos;
};
class Framebuffer {
public:
  static Ref<Framebuffer> Create(const FramebufferCreatInfo& info);
  Framebuffer(const FramebufferCreatInfo& info);
  ~Framebuffer();
  void bind(bool set_view_port = true) const;
  void unbind() const;

  void bind_texture(const std::string& name, int slot) const;
  void attach_layer_texture(int layer, const std::string& name);
  void add_attachment(const AttachmentInfo& attachment_info);

  void resize_attachment(const std::string& name, int width, int height);
  void resize_depth_renderbuffer(int width, int height);

  void clear();
private:
  void setup_depth_rbo();
  void setup_attachments();
  void invalidate();
  int get_slot(const Ref<Attachment>& attachment) const;

  uint32_t m_id{0};
  int m_width;
  int m_height;
  std::vector<AttachmentInfo> m_attachments_infos;
  std::unordered_map<std::string, Ref<Attachment>> m_attachments;
  std::vector<uint32_t> m_attachment_ids;
  uint32_t m_depth_rbo{0};

  const float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  const float ClearDepth    = 1.0f;
};
}  // namespace ezg::gl
#endif  //FRAMEBUFFER_HPP
