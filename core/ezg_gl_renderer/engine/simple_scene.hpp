#ifndef EASYGRAPHICS_SIMPLE_SCENE_HPP
#define EASYGRAPHICS_SIMPLE_SCENE_HPP

#include "scene.hpp"

namespace ezg::gl {
class SimpleScene : public BaseScene {
public:
  explicit SimpleScene(std::string_view name) : BaseScene(name) {}
  void load_new_model(uint32_t index) override;
  void load_floor() override;
  void load_light_model() override;
  void init() override;

  int get_num_models() override { return ModelNames.size(); }
  const char** get_model_data() override { return ModelNames.data(); }

private:
  const char* FloorPath{"../resources/models/wood_floor/scene.gltf"};
  const char* LightModelPath{"../resources/models/sun/scene.gltf"};
  std::vector<const char*> ModelNames{"ToyCar", "MetalRoughSpheres", "DamagedHelmet",
                                      "EnvironmentTest", "Sponza"};

  std::vector<const char*> ModelPaths{
      "../../glTF-Sample-Models/2.0/ToyCar/glTF/ToyCar.gltf",
      "../../glTF-Sample-Models/2.0/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf",
      "../../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf",
      "../../glTF-Sample-Models/2.0/EnvironmentTest/glTF/EnvironmentTest.gltf",
      "../../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"};
};
}  // namespace ezg::gl

#endif  //EASYGRAPHICS_SIMPLE_SCENE_HPP
