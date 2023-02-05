#include "basic_renderer.hpp"
#include "render_api.hpp"
namespace ezg::gl {

void BasicRenderer::render_models(const std::vector<ModelPtr>& models) {
  for (const auto& model : models) {
    RenderAPI::draw_model(model);
  }
}

void BasicRenderer::render_model(const ModelPtr& model) {
  RenderAPI::draw_model(model);
}
}