#ifndef EASYGRAPHICS_GLTF_UTILS_HPP
#define EASYGRAPHICS_GLTF_UTILS_HPP
#include <tiny_gltf.h>
#include <glm/glm.hpp>

namespace ezg::gl {
struct Vertex;
/**
 * Reference: https://github.com/vblanco20-1/vulkan-guide/blob/engine/asset-baker/asset_main.cpp
 */
void extract_gltf_indices(tinygltf::Primitive& primitive, tinygltf::Model& model,
                          std::vector<uint32_t>& indices);
/**
 * Reference: https://github.com/vblanco20-1/vulkan-guide/blob/engine/asset-baker/asset_main.cpp
 */
void extract_gltf_vertices(tinygltf::Primitive& primitive, tinygltf::Model& model,
                           std::vector<Vertex>& vertices);
/**
 * Reference: https://github.com/vblanco20-1/vulkan-guide/blob/engine/asset-baker/asset_main.cpp
 */
void unpack_gltf_buffer(tinygltf::Model& model, const tinygltf::Accessor& accessor,
                        std::vector<uint8_t>& out_buffer);

/**
 * Reference: https://gitlab.com/gltf-viewer-tutorial/gltf-viewer/-/blob/tutorial-v1/src/utils/gltf.cpp
 */
glm::mat4 getLocalToWorldMatrix(
    const tinygltf::Node &node, const glm::mat4 &parentMatrix);

/**
 * Reference: https://gitlab.com/gltf-viewer-tutorial/gltf-viewer/-/blob/tutorial-v1/src/utils/gltf.cpp
 */
void computeSceneBounds(
    const tinygltf::Model &model, glm::vec3 &bboxMin, glm::vec3 &bboxMax);
}
#endif  //EASYGRAPHICS_GLTF_UTILS_HPP
