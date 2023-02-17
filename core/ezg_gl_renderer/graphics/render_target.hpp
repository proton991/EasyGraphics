#ifndef EASYGRAPHICS_RENDER_TARGET_HPP
#define EASYGRAPHICS_RENDER_TARGET_HPP

#include <glad/glad.h>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace ezg::gl {
class RenderTarget;
using RenderTargetPtr = std::shared_ptr<RenderTarget>;
enum class RTAttachmentType : decltype(GL_COLOR_ATTACHMENT0) {
  COLOR0 = GL_COLOR_ATTACHMENT0,
  COLOR1 = GL_COLOR_ATTACHMENT1,
  COLOR2 = GL_COLOR_ATTACHMENT2,
  COLOR3 = GL_COLOR_ATTACHMENT3,
  COLOR4 = GL_COLOR_ATTACHMENT4,

  DEPTH   = GL_DEPTH_ATTACHMENT,
  STENCIL = GL_STENCIL_ATTACHMENT
};

enum class RTAttachmentFormat {
  None = 0,
  RGBA8,            // color attachment
  DEPTH24STENCIL8,  // depth attachment
};

struct RTAttachmentInfo {
  RTAttachmentInfo() = default;
  RTAttachmentInfo(std::string_view name_, RTAttachmentFormat format_)
      : format(format_), name(name_) {}
  RTAttachmentFormat format;
  std::string_view name;
};

struct RenderTargetInfo {
  uint32_t width{500};
  uint32_t height{500};
  uint32_t samples{1};
  // the index of attachment is used for texture unit when binding textures
  std::vector<RTAttachmentInfo> color_attachment_infos;
  bool has_depth{true};
};

class RenderTarget {
public:
  static RenderTargetPtr Create(const RenderTargetInfo& info);
  explicit RenderTarget(const RenderTargetInfo& rt_info);
  ~RenderTarget();

  void bind() const;
  void unbind();

  void resize(int width, int height);
  void invalidate();

  void bind_texture(std::string_view name);

private:
  void attach_color_texture(GLuint id, GLint internal_format, GLenum format, int index) const;
  void attach_depth_texture(uint32_t id, GLint format) const;

  GLuint m_id{0};
  RenderTargetInfo m_info;
  std::vector<GLuint> m_color_attachments;
  GLuint m_depth_attachment{0};
  bool m_multi_sampled{false};
  GLenum m_texture_target;
  std::unordered_map<std::string_view, int> m_name2index;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_RENDER_TARGET_HPP
