#include <glm/gtc/matrix_transform.hpp>
#include "engine.hpp"

namespace ege {
void EGEngine::RenderScene() {
  GPUCameraData camData;
  camData.proj     = m_camera.GetProjectionMatrix();
  camData.view     = m_camera.GetViewMatrix();
  camData.viewProj = camData.proj * camData.view;

  void* data;
  vmaMapMemory(m_allocator, GetCurrentFrame().cameraBuffer.m_allocation, &data);

  memcpy(data, &camData, sizeof(GPUCameraData));

  vmaUnmapMemory(m_allocator, GetCurrentFrame().cameraBuffer.m_allocation);

  Mesh* lastMesh         = nullptr;
  Material* lastMaterial = nullptr;

  for (const auto& sceneObj : m_sceneSystem.m_sceneObjs) {
    Material* material = m_sceneSystem.m_materials[sceneObj.materialId];
    Mesh* mesh         = m_sceneSystem.m_meshes[sceneObj.meshId];
    if (material != lastMaterial) {
      m_dispatchTable.fp_vkCmdBindPipeline(GetCurrentFrame().cmdBuffer,
                                           VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
      m_dispatchTable.fp_vkCmdBindDescriptorSets(
          GetCurrentFrame().cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0,
          1, &GetCurrentFrame().globalDescriptor, 0, nullptr);

      lastMaterial = material;
    }

    MeshPushConstants pushConstants{};
    pushConstants.renderMatrix = sceneObj.transformMatrix;
    m_dispatchTable.fp_vkCmdPushConstants(GetCurrentFrame().cmdBuffer, material->pipelineLayout,
                                          VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                                          &pushConstants);
    if (mesh != lastMesh) {
      VkDeviceSize offset = 0;
      m_dispatchTable.fp_vkCmdBindVertexBuffers(GetCurrentFrame().cmdBuffer, 0, 1,
                                                &mesh->m_vertexBuffer.m_buffer, &offset);
      lastMesh = mesh;
    }
    m_dispatchTable.fp_vkCmdDraw(GetCurrentFrame().cmdBuffer, mesh->m_vertices.size(), 1, 0, 0);
  }
}
}  // namespace ege