#include "resource_manager.hpp"
#include <spdlog/spdlog.h>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <fstream>
#include "utils/gltf_utils.hpp"

namespace ezg::gl {
std::string ResourceManager::load_shader_source(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::in);
  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if (!in) {
    spdlog::error("Failed to load shader source: {}", path.string());
    return "";
  }
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

ModelPtr ResourceManager::get_model(const std::string& name) {
  const auto iter = m_model_cache.find(name);
  if (iter == m_model_cache.end()) {
    spdlog::error("{} model not in cache", name);
    return nullptr;
  }
  return iter->second;
}

void ResourceManager::load_model(const std::string& name, const std::string& path,
                                 bool load_material) {
  spdlog::trace("Loading model {} from path {}", name, path);
  //attrib will contain the vertex arrays of the file
  tinyobj::attrib_t attrib;
  //shapes contains the info for each separate object in the file
  std::vector<tinyobj::shape_t> shapes;
  //materials contains the information about the material of each shape, but we wont use it.
  std::vector<tinyobj::material_t> materials;

  //error and warning output from the load function
  std::string warn;
  std::string err;

  //load the OBJ file
  tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr);
  //make sure to output the warnings to the console, in case there are issues with the file
  if (!warn.empty()) {
    spdlog::warn(warn);
  }
  //if we have any error, print it to the console, and break the mesh loading.
  //This happens if the file cant be found or is malformed
  if (!err.empty()) {
    spdlog::error(err);
    return;
  }

  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

      //hardcode loading to triangles
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        //vertex position
        tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
        tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
        tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
        //vertex normal
        tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
        tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
        tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

        //vertex uv
        tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
        tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

        //copy it into our vertex
        Vertex new_vert;
        new_vert.position.x = vx;
        new_vert.position.y = vy;
        new_vert.position.z = vz;

        new_vert.normal.x = nx;
        new_vert.normal.y = ny;
        new_vert.normal.z = nz;

        new_vert.uv.x = ux;
        new_vert.uv.y = 1 - uy;

        //we are setting the vertex color as the vertex normal. This is just for display purposes

        indices.push_back(static_cast<GLuint>(vertices.size()));
        vertices.push_back(new_vert);
      }
      index_offset += fv;
    }
  }

  // TODO: load material
  if (load_material) {}
  // cache material
  m_model_cache.try_emplace(name, std::make_shared<Model>(name, vertices, indices));
}

ModelPtr ResourceManager::load_gltf_model(const std::string& name, const std::string& path) {
  tinygltf::Model gltf_model;
  tinygltf::TinyGLTF loader;
  std::string error;
  std::string warning;
  bool ret = loader.LoadASCIIFromFile(&gltf_model, &error, &warning, path.c_str());
  if (!warning.empty()) {
    spdlog::warn(warning);
  }
  if (!error.empty()) {
    spdlog::error(error);
  }
  if (!ret) {
    spdlog::error("Failed to parse glTF");
    return nullptr;
  }
  const auto load_textures = [&](const tinygltf::Model& model) {
    tinygltf::Sampler defaultSampler;
    defaultSampler.minFilter = GL_LINEAR;
    defaultSampler.magFilter = GL_LINEAR;
    defaultSampler.wrapS     = GL_REPEAT;
    defaultSampler.wrapT     = GL_REPEAT;
    for (size_t i = 0; i < model.textures.size(); i++) {
      const auto& texture = model.textures[i];
      const auto& image   = model.images[texture.source];
      const auto& sampler = texture.sampler >= 0 ? model.samplers[texture.sampler] : defaultSampler;
      TextureInfo info{image.width,       image.height,  sampler.minFilter,
                       sampler.magFilter, sampler.wrapS, sampler.wrapT};
      if (sampler.minFilter == GL_NEAREST_MIPMAP_NEAREST ||
          sampler.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
          sampler.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
          sampler.minFilter == GL_LINEAR_MIPMAP_LINEAR) {
        info.generate_mipmap = true;
      }
      m_texture_cache.emplace_back(Texture2D::Create(info, image.image.data()));
    }
  };

  const auto bind_material = [&](const auto materialIndex, Mesh& mesh) {
    PBRMaterial mesh_material{};
    if (materialIndex >= 0) {
      const tinygltf::Material& material = gltf_model.materials[materialIndex];

      const auto& pbrMetallicRoughness = material.pbrMetallicRoughness;
      mesh_material.base_color_factor  = {(float)pbrMetallicRoughness.baseColorFactor[0],
                                         (float)pbrMetallicRoughness.baseColorFactor[1],
                                         (float)pbrMetallicRoughness.baseColorFactor[2],
                                         (float)pbrMetallicRoughness.baseColorFactor[3]};
      if (pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const auto& baseColorTexture =
            gltf_model.textures[pbrMetallicRoughness.baseColorTexture.index];
        if (baseColorTexture.source >= 0) {
          mesh_material.textures[PBRComponent::BaseColor] =
              m_texture_cache[baseColorTexture.source];
        }
      }

      if (pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        const auto& metallicRoughnessTexture =
            gltf_model.textures[pbrMetallicRoughness.metallicRoughnessTexture.index];
        if (metallicRoughnessTexture.source >= 0) {
          mesh_material.textures[PBRComponent::MetallicRoughness] =
              m_texture_cache[metallicRoughnessTexture.source];
        }
      }

      if (material.emissiveTexture.index >= 0) {
        const auto& emissiveTexture = gltf_model.textures[material.emissiveTexture.index];
        if (emissiveTexture.source >= 0) {
          mesh_material.textures[PBRComponent::Emissive] = m_texture_cache[emissiveTexture.source];
        }
      }

      if (material.occlusionTexture.index >= 0) {
        const auto& occlusionTexture = gltf_model.textures[material.occlusionTexture.index];
        if (occlusionTexture.source >= 0) {
          mesh_material.textures[PBRComponent::Occlusion] =
              m_texture_cache[occlusionTexture.source];
        }
      }

    } else {
      // Apply default material
      // Defined here:
      // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#reference-material
      // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#reference-pbrmetallicroughness3

      mesh_material.textures[PBRComponent::BaseColor] = m_white_texture;
    }
    mesh.material = std::move(mesh_material);
  };
  load_textures(gltf_model);
  std::unordered_map<int, glm::mat4> mesh_matrices;
  const std::function<void(int, const glm::mat4&)> extract_node_matrices =
      [&](int node_idx, const glm::mat4& parent_matrix) {
        const auto& node = gltf_model.nodes[node_idx];
        // get world matrix
        const glm::mat4 model_matrix = getLocalToWorldMatrix(node, parent_matrix);
        if (node.mesh >= 0) {
          mesh_matrices[node.mesh] = model_matrix;
        }
        for (const auto child : node.children) {
          extract_node_matrices(child, model_matrix);
        }
      };
  if (gltf_model.defaultScene >= 0) {
    for (const auto node_idx : gltf_model.scenes[gltf_model.defaultScene].nodes) {
      extract_node_matrices(node_idx, glm::mat4(1.0f));
    }
  }
  auto model = Model::Create(name);
  spdlog::info("Model {} mesh size : {}", name, gltf_model.meshes.size());
  for (auto mesh_idx = 0; mesh_idx < gltf_model.meshes.size(); mesh_idx++) {
    auto& gl_mesh = gltf_model.meshes[mesh_idx];
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (auto primitive_index = 0; primitive_index < gl_mesh.primitives.size(); primitive_index++) {
      vertices.clear();
      indices.clear();
      auto& primitive = gl_mesh.primitives[primitive_index];
      extract_gltf_vertices(primitive, gltf_model, vertices);
      extract_gltf_indices(primitive, gltf_model, indices);
      Mesh mesh{vertices, indices};
      if (mesh_matrices.find(mesh_idx) != mesh_matrices.end()) {
        mesh.model_matrix = mesh_matrices[mesh_idx];
      }
      bind_material(primitive.material, mesh);
      model->attach_mesh(mesh);
    }
  }

  glm::vec3 bbox_min, bbox_max;
  // compute boundary
  computeSceneBounds(gltf_model, bbox_min, bbox_max);
  AABB aabb{bbox_min, bbox_max};
  model->set_aabb(aabb);
  m_model_cache[name] = model;
  return model;
}

}  // namespace ezg::gl