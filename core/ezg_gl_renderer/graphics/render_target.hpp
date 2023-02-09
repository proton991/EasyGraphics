#ifndef EASYGRAPHICS_RENDER_TARGET_HPP
#define EASYGRAPHICS_RENDER_TARGET_HPP

#include <glad/glad.h>
#include <memory>
#include <vector>
namespace ezg::gl {
struct Texture;
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
  RTAttachmentInfo(RTAttachmentFormat format_) : format(format_) {}
  RTAttachmentFormat format;
};

struct RenderTargetInfo {
  int width{500};
  int height{500};
  int samples{1};
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

private:
  void attach_color_texture(GLuint id, GLint internal_format, GLenum format, int index) const;
  void attach_depth_texture(uint32_t id, GLint format) const;
  void bind_texture(GLuint id);
  void create_textures(GLuint* ids, int count);

  GLuint m_id{0};
  RenderTargetInfo m_info;
  std::vector<GLuint> m_color_attachments;
  GLuint m_depth_attachment{0};
  bool m_multi_sampled{false};
  GLenum m_texture_target;
};
}  // namespace ezg::gl
#endif  //EASYGRAPHICS_RENDER_TARGET_HPP
