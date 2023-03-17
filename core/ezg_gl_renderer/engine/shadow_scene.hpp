#ifndef EASYGRAPHICS_SHADOW_SCENE_HPP
#define EASYGRAPHICS_SHADOW_SCENE_HPP
#include "scene.hpp"

namespace ezg::gl {
class ShadowScene : public BaseScene {
public:
  explicit ShadowScene(std::string_view name) : BaseScene(name) {}
  void load_new_model(uint32_t index) override;
  void load_floor() override;
  void load_light_model() override;
  void init() override;

  int get_num_models() override { return ModelNames.size(); }
  const char** get_model_data() override { return ModelNames.data(); }

private:
  const char* FloorPath{"../resources/models/wood_floor/scene.gltf"};
  const char* LightModelPath{"../resources/models/sun/scene.gltf"};
  std::vector<const char*> ModelNames{"Box", "DamagedHelmet", "FlightHelmet"};

  std::vector<const char*> ModelPaths{
      "../../glTF-Sample-Models/2.0/Box/glTF/Box.gltf",
      "../../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf",
      "../../glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf"
  };
};
}  // namespace ezg::gl

#endif  //EASYGRAPHICS_SHADOW_SCENE_HPP
