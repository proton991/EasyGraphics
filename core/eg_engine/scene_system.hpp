#ifndef SCENE_SYSTEM_HPP
#define SCENE_SYSTEM_HPP

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "types.hpp"

namespace ege {

struct Mesh;
struct Material;
struct SceneObjectInfo {
  Mesh* mesh{};
  Material* material{};
  glm::mat4 transformMatrix{1.0f};
};

struct SceneObject {
  ID_TYPE meshId{};
  ID_TYPE materialId{};
  glm::mat4 transformMatrix{1.0f};
};
class SceneSystem {
public:
  friend class EGEngine;
  void Init();

  void AddObject(SceneObjectInfo* info);

  void AddObjectBatch(SceneObjectInfo* first, uint32_t count);

private:
  ID_TYPE GetMeshId(Mesh* mesh);
  ID_TYPE GetMaterialId(Material* material);

  std::vector<SceneObject> m_sceneObjs;

  std::vector<Mesh*> m_meshes;
  std::vector<Material*> m_materials;

  std::unordered_map<Mesh*, ID_TYPE> m_meshMap;
  std::unordered_map<Material*, ID_TYPE> m_materialMap;
};
}  // namespace ege
#endif  //SCENE_SYSTEM_HPP
