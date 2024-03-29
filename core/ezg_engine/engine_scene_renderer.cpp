#include <glm/gtc/matrix_transform.hpp>
#include "engine.hpp"

namespace ezg {
void EGEngine::RenderScene() {
  GPUCameraData camData;
  camData.proj     = m_camera.GetProjectionMatrix();
  camData.view     = m_camera.GetViewMatrix();
  camData.viewProj = camData.proj * camData.view;

  void* data;
  vmaMapMemory(m_allocator, GetCurrentFrame().cameraBuffer.m_allocation, &data);

  memcpy(data, &camData, sizeof(GPUCameraData));

  vmaUnmapMemory(m_allocator, GetCurrentFrame().cameraBuffer.m_allocation);

  float framed = (m_frameNumber / 120.f);

  m_sceneParameters.ambientColor = {sin(framed), 0, cos(framed), 1};

  char* sceneData;
  vmaMapMemory(m_allocator, m_sceneParameterBuffer.m_allocation, (void**)&sceneData);
  int frameIndex = m_frameNumber % FRAME_OVERLAP;

  uint32_t uniformOffset = PadUniformBufferSize(sizeof(GPUSceneData)) * frameIndex;
  sceneData += uniformOffset;
  memcpy(sceneData, &m_sceneParameters, sizeof(GPUSceneData));
  vmaUnmapMemory(m_allocator, m_sceneParameterBuffer.m_allocation);

  void* objectData;
  vmaMapMemory(m_allocator, GetCurrentFrame().objectBuffer.m_allocation, &objectData);

  GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

  for (int i = 0; i < m_sceneSystem.m_sceneObjs.size(); i++) {
    auto& object              = m_sceneSystem.m_sceneObjs[i];
    objectSSBO[i].modelMatrix = object.transformMatrix;
  }

  vmaUnmapMemory(m_allocator, GetCurrentFrame().objectBuffer.m_allocation);
  Mesh* lastMesh         = nullptr;
  Material* lastMaterial = nullptr;

  for (int i = 0; i < m_sceneSystem.m_sceneObjs.size(); i++) {
    const auto& sceneObj = m_sceneSystem.m_sceneObjs[i];
    Material* material   = m_sceneSystem.m_materials[sceneObj.materialId];
    Mesh* mesh           = m_sceneSystem.m_meshes[sceneObj.meshId];
    if (material != lastMaterial) {
      m_dispatchTable.cmdBindPipeline(GetCurrentFrame().cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      material->pipeline);
      m_dispatchTable.cmdBindDescriptorSets(
          GetCurrentFrame().cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0,
          1, &GetCurrentFrame().globalDescriptor, 1, &uniformOffset);

      m_dispatchTable.cmdBindDescriptorSets(
          GetCurrentFrame().cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 1,
          1, &GetCurrentFrame().objectDescriptor, 0, nullptr);

      if (material->textureSet != VK_NULL_HANDLE) {
        m_dispatchTable.cmdBindDescriptorSets(
            GetCurrentFrame().cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout,
            2, 1, &material->textureSet, 0, nullptr);
      }
      lastMaterial = material;
    }

    MeshPushConstants pushConstants{};
    pushConstants.renderMatrix = sceneObj.transformMatrix;
    m_dispatchTable.cmdPushConstants(GetCurrentFrame().cmdBuffer, material->pipelineLayout,
                                     VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                                     &pushConstants);
    if (mesh != lastMesh) {
      VkDeviceSize offset = 0;
      m_dispatchTable.cmdBindVertexBuffers(GetCurrentFrame().cmdBuffer, 0, 1,
                                           &mesh->m_vertexBuffer.m_buffer, &offset);
      lastMesh = mesh;
    }
    m_dispatchTable.cmdDraw(GetCurrentFrame().cmdBuffer, mesh->m_vertices.size(), 1, 0, i);
  }
}
}  // namespace ezg