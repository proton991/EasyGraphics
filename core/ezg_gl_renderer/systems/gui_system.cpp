#include "gui_system.hpp"
#include "log.hpp"

namespace ezg::system {
const char* glsl_version = "#version 450";
Ref<GUISystem> GUISystem::Create(GLFWwindow* glfw_window) {
  return CreateRef<GUISystem>(glfw_window);
}

GUISystem::~GUISystem() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

GUISystem::GUISystem(GLFWwindow* glfw_window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

void GUISystem::draw(Ref<gl::RenderOptions> options) {
  begin_frame();
  {
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("GUI");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
    ImGui::Text("Frame time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::PopStyleColor();

    ImGui::Checkbox("Show Axis", &options->show_axis);
    ImGui::SameLine();
    ImGui::Checkbox("Show AABB", &options->show_aabb);

    options->scene_changed = ImGui::Combo("Select Model", &options->selected_model,
                                          options->model_list, options->num_models);
    ImGui::Checkbox("Rotate Model", &options->rotate_model);
    ImGui::SameLine();
    ImGui::Checkbox("Rotate Camera", &options->rotate_camera);

    if (ImGui::CollapsingHeader("Environment")) {
      ImGui::Checkbox("Enable Environment Map", &options->enable_env_map);
      ImGui::Checkbox("Show Background", &options->show_bg);
      ImGui::SameLine();
      ImGui::Checkbox("Blur", &options->blur);
    }
    ImGui::End();
  }
  end_frame();
  render_frame();
}

void GUISystem::begin_frame() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void GUISystem::end_frame() {
  ImGui::EndFrame();
}

void GUISystem::render_frame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace ezg::system