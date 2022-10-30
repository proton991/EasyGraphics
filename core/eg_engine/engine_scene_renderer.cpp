#include <glm/gtc/matrix_transform.hpp>
#include "engine.hpp"

namespace ege {
void EGEngine::RenderScene() {
  Mesh* lastMesh         = nullptr;
  Material* lastMaterial = nullptr;
  glm::mat4 model;
  for (const auto& sceneObj : m_sceneSystem.m_sceneObjs) {
    Material* material = m_sceneSystem.m_materials[sceneObj.materialId];
    Mesh* mesh         = m_sceneSystem.m_meshes[sceneObj.meshId];
    if (material != lastMaterial) {
      m_dispatchTable.fp_vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                           material->pipeline);
      lastMaterial = material;
    }
    model = sceneObj.transformMatrix;
    glm::mat4 meshMatrix = m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix() * model;
    MeshPushConstants pushConstants{};
    pushConstants.renderMatrix = meshMatrix;
    m_dispatchTable.fp_vkCmdPushConstants(m_cmdBuffer, material->pipelineLayout,
                                          VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                                          &pushConstants);
    if (mesh != lastMesh) {
      VkDeviceSize offset = 0;
      m_dispatchTable.fp_vkCmdBindVertexBuffers(m_cmdBuffer, 0, 1, &mesh->m_vertexBuffer.m_buffer, &offset);
      lastMesh = mesh;
    }
    m_dispatchTable.fp_vkCmdDraw(m_cmdBuffer, mesh->m_vertices.size(), 1, 0, 0);
  }
}
}  // namespace ege