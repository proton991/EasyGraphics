#include "gltf_utils.hpp"
#include "log.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <vector>
#include "assets/mesh.hpp"

/**
 * Reference: https://gitlab.com/gltf-viewer-tutorial/gltf-viewer
 */

namespace ezg::gl {
void extract_gltf_vertices(tinygltf::Primitive& primitive, tinygltf::Model& model,
                           std::vector<Vertex>& vertices) {
  tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes["POSITION"]];

  vertices.resize(pos_accessor.count);

  std::vector<uint8_t> pos_data;
  unpack_gltf_buffer(model, pos_accessor, pos_data);

  for (int i = 0; i < vertices.size(); i++) {
    if (pos_accessor.type == TINYGLTF_TYPE_VEC3) {
      if (pos_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        float* dtf = (float*)pos_data.data();

        //vec3f
        vertices[i].position[0] = *(dtf + (i * 3) + 0);
        vertices[i].position[1] = *(dtf + (i * 3) + 1);
        vertices[i].position[2] = *(dtf + (i * 3) + 2);
      } else {
        assert(false);
      }
    } else {
      assert(false);
    }
  }

  const auto iterator = primitive.attributes.find("TEXCOORD_0");
  if (iterator != primitive.attributes.end()) {
    tinygltf::Accessor& uv_accessor = model.accessors[iterator->second];

    std::vector<uint8_t> uv_data;
    unpack_gltf_buffer(model, uv_accessor, uv_data);

    for (int i = 0; i < vertices.size(); i++) {
      if (uv_accessor.type == TINYGLTF_TYPE_VEC2) {
        if (uv_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
          float* dtf = (float*)uv_data.data();

          //vec3f
          vertices[i].uv[0] = *(dtf + (i * 2) + 0);
          vertices[i].uv[1] = *(dtf + (i * 2) + 1);
        } else {
          assert(false);
        }
      } else {
        assert(false);
      }
    }
  }

  tinygltf::Accessor& normal_accessor = model.accessors[primitive.attributes["NORMAL"]];

  std::vector<uint8_t> normal_data;
  unpack_gltf_buffer(model, normal_accessor, normal_data);

  for (int i = 0; i < vertices.size(); i++) {
    if (normal_accessor.type == TINYGLTF_TYPE_VEC3) {
      if (normal_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        float* dtf = (float*)normal_data.data();
        //vec3f
        vertices[i].normal[0] = *(dtf + (i * 3) + 0);
        vertices[i].normal[1] = *(dtf + (i * 3) + 1);
        vertices[i].normal[2] = *(dtf + (i * 3) + 2);

        //        _vertices[i].color[0] = *(dtf + (i * 3) + 0);
        //        _vertices[i].color[1] = *(dtf + (i * 3) + 1);
        //        _vertices[i].color[2] = *(dtf + (i * 3) + 2);
      } else {
        assert(false);
      }
    } else {
      assert(false);
    }
  }

  tinygltf::Accessor& tangent_accessor = model.accessors[primitive.attributes["TANGENT"]];

  std::vector<uint8_t> tangent_data;
  unpack_gltf_buffer(model, tangent_accessor, tangent_data);

  //  for (int i = 0; i < vertices.size(); i++) {
  //    if (tangent_accessor.type == TINYGLTF_TYPE_VEC3) {
  //      if (tangent_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
  //        float* dtf = (float*)tangent_data.data();
  //        //vec3f
  //        vertices[i].tangent[0] = *(dtf + (i * 3) + 0);
  //        vertices[i].tangent[1] = *(dtf + (i * 3) + 1);
  //        vertices[i].tangent[2] = *(dtf + (i * 3) + 2);
  //      } else {
  //        assert(false);
  //      }
  //    } else {
  //      assert(false);
  //    }
  //  }
}

void extract_gltf_indices(tinygltf::Primitive& primitive, tinygltf::Model& model,
                          std::vector<uint32_t>& indices) {
  int accessor_index       = primitive.indices;
  const auto& accessor     = model.accessors[accessor_index];
  const int component_type = accessor.componentType;
  std::vector<uint8_t> unpacked_indices;
  unpack_gltf_buffer(model, accessor, unpacked_indices);
  for (int i = 0; i < accessor.count; i++) {
    uint32_t index;
    switch (component_type) {
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
        auto* buf = reinterpret_cast<uint16_t*>(unpacked_indices.data());
        index     = *(buf + i);
        break;
      }
      case TINYGLTF_COMPONENT_TYPE_SHORT: {
        auto* buf = reinterpret_cast<int16_t*>(unpacked_indices.data());
        index     = *(buf + i);
        break;
      }
      case TINYGLTF_COMPONENT_TYPE_INT: {
        auto* buf = reinterpret_cast<int32_t*>(unpacked_indices.data());
        index     = *(buf + i);
        break;
      }
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
        auto* buf = reinterpret_cast<uint32_t*>(unpacked_indices.data());
        index     = *(buf + i);
        break;
      }
      default: {
        spdlog::error("Invalid component type for indices");
        assert(false);
      }
    }
    indices.push_back(index);
  }
  // flip the triangle
  for (int i = 0; i < indices.size() / 3; ++i) {
    std::swap(indices[i * 3 + 1], indices[i * 3 + 2]);
  }
}

void unpack_gltf_buffer(tinygltf::Model& model, const tinygltf::Accessor& accessor,
                        std::vector<uint8_t>& out_buffer) {
  tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
  tinygltf::Buffer& buffer          = model.buffers[buffer_view.buffer];

  uint8_t* data_ptr = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

  size_t element_size = tinygltf::GetComponentSizeInBytes(accessor.componentType) *
                        tinygltf::GetNumComponentsInType(accessor.type);
  size_t stride = buffer_view.byteStride;
  if (stride == 0) {
    stride = element_size;
  }
  out_buffer.resize(accessor.count * element_size);

  for (int i = 0; i < accessor.count; i++) {
    uint8_t* src = data_ptr + stride * i;
    uint8_t* dst = out_buffer.data() + element_size * i;
    memcpy(dst, src, element_size);
  }
}

glm::mat4 getLocalToWorldMatrix(const tinygltf::Node& node, const glm::mat4& parentMatrix) {
  // Extract model matrix
  // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#transformations
  if (!node.matrix.empty()) {
    return parentMatrix * glm::mat4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                                    node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                                    node.matrix[8], node.matrix[9], node.matrix[10],
                                    node.matrix[11], node.matrix[12], node.matrix[13],
                                    node.matrix[14], node.matrix[15]);
  }
  const auto T =
      node.translation.empty()
          ? parentMatrix
          : glm::translate(parentMatrix, glm::vec3(node.translation[0], node.translation[1],
                                                   node.translation[2]));
  const auto rotationQuat =
      node.rotation.empty()
          ? glm::quat(1, 0, 0, 0)
          : glm::quat(float(node.rotation[3]), float(node.rotation[0]), float(node.rotation[1]),
                      float(node.rotation[2]));  // prototype is w, x, y, z
  const auto TR = T * glm::mat4_cast(rotationQuat);
  return node.scale.empty()
             ? TR
             : glm::scale(TR, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
};

void computeSceneBounds(const tinygltf::Model& model, glm::vec3& bboxMin, glm::vec3& bboxMax) {
  // Compute scene bounding box
  // todo refactor with scene drawing
  // todo need a visitScene generic function that takes a accept() functor
  bboxMin = glm::vec3(std::numeric_limits<float>::max());
  bboxMax = glm::vec3(std::numeric_limits<float>::lowest());
  if (model.defaultScene >= 0) {
    const std::function<void(int, const glm::mat4&)> updateBounds = [&](int nodeIdx,
                                                                        const glm::mat4&
                                                                            parentMatrix) {
      const auto& node = model.nodes[nodeIdx];
      // model matrix
      const glm::mat4 modelMatrix = getLocalToWorldMatrix(node, parentMatrix);
      if (node.mesh >= 0) {
        const auto& mesh = model.meshes[node.mesh];
        for (size_t pIdx = 0; pIdx < mesh.primitives.size(); ++pIdx) {
          const auto& primitive        = mesh.primitives[pIdx];
          const auto positionAttrIdxIt = primitive.attributes.find("POSITION");
          if (positionAttrIdxIt == end(primitive.attributes)) {
            continue;
          }
          const auto& positionAccessor = model.accessors[(*positionAttrIdxIt).second];
          if (positionAccessor.type != 3) {
            std::cerr << "Position accessor with type != VEC3, skipping" << std::endl;
            continue;
          }
          const auto& positionBufferView = model.bufferViews[positionAccessor.bufferView];
          const auto byteOffset      = positionAccessor.byteOffset + positionBufferView.byteOffset;
          const auto& positionBuffer = model.buffers[positionBufferView.buffer];
          const auto positionByteStride =
              positionBufferView.byteStride ? positionBufferView.byteStride : 3 * sizeof(float);

          if (primitive.indices >= 0) {
            const auto& indexAccessor   = model.accessors[primitive.indices];
            const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const auto indexByteOffset  = indexAccessor.byteOffset + indexBufferView.byteOffset;
            const auto& indexBuffer     = model.buffers[indexBufferView.buffer];
            auto indexByteStride        = indexBufferView.byteStride;

            switch (indexAccessor.componentType) {
              default:
                std::cerr << "Primitive index accessor with bad componentType "
                          << indexAccessor.componentType << ", skipping it." << std::endl;
                continue;
              case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                indexByteStride = indexByteStride ? indexByteStride : sizeof(uint8_t);
                break;
              case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                indexByteStride = indexByteStride ? indexByteStride : sizeof(uint16_t);
                break;
              case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                indexByteStride = indexByteStride ? indexByteStride : sizeof(uint32_t);
                break;
            }

            for (size_t i = 0; i < indexAccessor.count; ++i) {
              uint32_t index = 0;
              switch (indexAccessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                  index =
                      *((const uint8_t*)&indexBuffer.data[indexByteOffset + indexByteStride * i]);
                  break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                  index =
                      *((const uint16_t*)&indexBuffer.data[indexByteOffset + indexByteStride * i]);
                  break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                  index =
                      *((const uint32_t*)&indexBuffer.data[indexByteOffset + indexByteStride * i]);
                  break;
              }
              const auto& localPosition = *(
                  (const glm::vec3*)&positionBuffer.data[byteOffset + positionByteStride * index]);
              const auto worldPosition = glm::vec3(modelMatrix * glm::vec4(localPosition, 1.f));
              bboxMin                  = glm::min(bboxMin, worldPosition);
              bboxMax                  = glm::max(bboxMax, worldPosition);
            }
          } else {
            for (size_t i = 0; i < positionAccessor.count; ++i) {
              const auto& localPosition =
                  *((const glm::vec3*)&positionBuffer.data[byteOffset + positionByteStride * i]);
              const auto worldPosition = glm::vec3(modelMatrix * glm::vec4(localPosition, 1.f));
              bboxMin                  = glm::min(bboxMin, worldPosition);
              bboxMax                  = glm::max(bboxMax, worldPosition);
            }
          }
        }
      }
      for (const auto childNodeIdx : node.children) {
        updateBounds(childNodeIdx, modelMatrix);
      }
    };
    for (const auto nodeIdx : model.scenes[model.defaultScene].nodes) {
      updateBounds(nodeIdx, glm::mat4(1));
    }
  }
}
}  // namespace ezg::gl