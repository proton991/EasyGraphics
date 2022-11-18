#include "scene_system.hpp"
#include <glm/ext/matrix_transform.hpp>
#include "engine.hpp"
#include "mesh.hpp"

namespace ezg {
void SceneSystem::Init() {}

//void SceneSystem::LoadDefaultScene() {
//  Mesh quadMesh{};
//  quadMesh.LoadFromObj("../assets/quad.obj");
//  m_engine->UploadMesh(quadMesh);
//  glm::mat4 quad = glm::scale(glm::mat4{1.f}, {3.f, 1.f, 3.f});
//  glm::translate(quad, {0.f, 0.5f, 0.f});
//  SceneObjectInfo quadObjInfo{};
//  quadObjInfo.material        = m_engine->m_materialSystem.GetMaterial("default");
//  quadObjInfo.mesh            = &quadMesh;
//  quadObjInfo.transformMatrix = quad;
//  AddObject(&quadObjInfo);
//
////
////
//  Mesh monkeyMesh{};
//  monkeyMesh.LoadFromObj("../assets/smooth_vase.obj");
//  m_engine->UploadMesh(monkeyMesh);
//  glm::mat4 monkey = glm::scale(glm::mat4{1.0f}, {5, 5, 5});
//  glm::translate(monkey, {0, 0, 0});
//  SceneObjectInfo monkeyObjInfo{};
//  monkeyObjInfo.material        = m_engine->m_materialSystem.GetMaterial("default");
//  monkeyObjInfo.mesh            = &monkeyMesh;
//  monkeyObjInfo.transformMatrix = monkey;
//  AddObject(&monkeyObjInfo);
//
//
//}

void SceneSystem::AddObject(SceneObjectInfo* info) {
  SceneObject newObj;
  newObj.transformMatrix = info->transformMatrix;
  newObj.materialId      = GetMaterialId(info->material);
  newObj.meshId          = GetMeshId(info->mesh);
  m_sceneObjs.push_back(newObj);
}

void SceneSystem::AddObjectBatch(SceneObjectInfo* first, uint32_t count) {
  m_sceneObjs.reserve(count);
  for (uint32_t i = 0; i < count; i++) {
    AddObject(&first[i]);
  }
}

ID_TYPE SceneSystem::GetMeshId(Mesh* mesh) {
  auto it = m_meshMap.find(mesh);
  ID_TYPE id;
  if (it == m_meshMap.end()) {
    id              = m_meshes.size();
    m_meshMap[mesh] = id;
    m_meshes.push_back(mesh);
  } else {
    id = it->second;
  }
  return id;
}

ID_TYPE SceneSystem::GetMaterialId(Material* material) {
  auto it = m_materialMap.find(material);
  ID_TYPE id;
  if (it == m_materialMap.end()) {
    id                      = m_materials.size();
    m_materialMap[material] = id;
    m_materials.push_back(material);
  } else {
    id = it->second;
  }
  return id;
}
}  // namespace ezg