#include "resource_manager.hpp"
#include <spdlog/spdlog.h>
#include <tiny_obj_loader.h>
#include <tiny_gltf.h>
#include <fstream>

namespace ezg::gl {
void extract_gltf_indices(tinygltf::Primitive& primitive, tinygltf::Model& model,
                          std::vector<uint32_t>& indices);
void extract_gltf_vertices(tinygltf::Primitive& primitive, tinygltf::Model& model,
                           std::vector<Vertex>& vertices);
void unpack_gltf_buffer(tinygltf::Model& model, const tinygltf::Accessor& accessor,
                        std::vector<uint8_t>& out_buffer);

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

  tinygltf::Accessor& uv_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];

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
  auto model = Model::Create(name);
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
      model->attach_mesh(mesh);
    }
  }
  m_model_cache[name] = model;
  return model;
}

}  // namespace ezg::gl