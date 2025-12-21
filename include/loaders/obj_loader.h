#ifndef VOXL_OBJ_LOADER_H
#define VOXL_OBJ_LOADER_H


#include <cstddef>
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include <memory>

#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "components/mesh.h"
#include "components/mesh.h"


struct TinyObjIndexComp 
{
  bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const 
  {
    if (a.vertex_index != b.vertex_index) return a.vertex_index < b.vertex_index;
    if (a.normal_index != b.normal_index) return a.normal_index < b.normal_index;
    return a.texcoord_index < b.texcoord_index;
  }
};


inline Mesh LoadOBJ(const std::string& name, const glm::vec4& color = glm::vec4(1.0f));


struct OBJLoader
{
  using result_type = std::shared_ptr<Mesh>;

  result_type operator()(const std::string& name, const glm::vec4& color = glm::vec4(1.0f))
  {
    Mesh mesh = LoadOBJ(name, color);
    return std::make_shared<Mesh>(mesh);
  }
};


inline Mesh LoadOBJ(const std::string& name, const glm::vec4& color)
{
  std::string obj_path = "assets/models/" + name + ".obj";
  std::string mtl_path = "assets/models/";

  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = mtl_path;
  reader_config.triangulate = true;

  tinyobj::ObjReader obj_reader;

  if (!obj_reader.ParseFromFile(obj_path, reader_config)) 
  {
    if (!obj_reader.Error().empty()) 
    {
      std::cerr << "[LoadOBJ] " << obj_reader.Error();
    }
    return Mesh{};
  }

  if (!obj_reader.Warning().empty()) 
  {
    std::cout << "[LoadOBJ] " << obj_reader.Warning();
  }

  auto& attrib = obj_reader.GetAttrib(); // contains la tableau des sommets
  auto& shapes = obj_reader.GetShapes(); // contient les infos de tous les objets dans le fichier
  auto& materials = obj_reader.GetMaterials(); // inutile pour le moment

  Mesh mesh;
  std::map<tinyobj::index_t, unsigned int, TinyObjIndexComp> unique_vertices;

  for (const auto& shape : shapes) 
  {
    size_t index_offset = 0;

    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) 
    {
      int face_vertices = shape.mesh.num_face_vertices[f];

      for (size_t v = 0; v < face_vertices; v++) 
      {
        tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

        // on ajoute l'indice du sommets si celui-ci existe déjà
        if (unique_vertices.count(idx) > 0)
        {
          mesh.indices.push_back(unique_vertices[idx]);
        }
        else 
        {
          // position
          tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
          tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
          tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

          tinyobj::real_t nx = 0.0f, ny = 0.0f, nz = 1.0f;
          tinyobj::real_t tu = 0.0f, tv = 0.0f;

          // normal
          if (idx.normal_index >= 0)
          {
            nx = attrib.normals[3 * idx.normal_index + 0];
            ny = attrib.normals[3 * idx.normal_index + 1];
            nz = attrib.normals[3 * idx.normal_index + 2];
          }

          // textureCoordinates
          if (idx.texcoord_index >= 0)
          {
            tu = attrib.texcoords[2 * idx.texcoord_index + 0];
            tv = attrib.texcoords[2 * idx.texcoord_index + 1];
          }

          Vertex mesh_vertex;
          mesh_vertex.position = {vx, vy, vz};
          mesh_vertex.normal = {nx, ny, nz};
          mesh_vertex.textureCoordinates = {tu, tv};
          mesh_vertex.color = color;
          
          mesh.vertices.push_back(mesh_vertex);

          unsigned int new_index = static_cast<unsigned int>(mesh.vertices.size() - 1);

          unique_vertices[idx] = new_index;
          mesh.indices.push_back(new_index);
        }
      }
      index_offset += face_vertices;
    }
  }

  mesh.indiceCount = mesh.indices.size();

  glCreateVertexArrays(1, &mesh.vao);
  glCreateBuffers(1, &mesh.vbo);
  glCreateBuffers(1, &mesh.ebo);

  glNamedBufferStorage(mesh.vbo, sizeof(Vertex) * mesh.vertices.size(), mesh.vertices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(Vertex));

  glNamedBufferStorage(mesh.ebo, sizeof(unsigned int) * mesh.indiceCount, mesh.indices.data(), GL_DYNAMIC_STORAGE_BIT);
  glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

  glEnableVertexArrayAttrib(mesh.vao, 0); // position => 0
  glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribBinding(mesh.vao, 0, 0);

  glEnableVertexArrayAttrib(mesh.vao, 1); // normal => 1
  glVertexArrayAttribFormat(mesh.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribBinding(mesh.vao, 1, 0);

  glEnableVertexArrayAttrib(mesh.vao, 2); // textureCoordinates => 2
  glVertexArrayAttribFormat(mesh.vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureCoordinates));
  glVertexArrayAttribBinding(mesh.vao, 2, 0);

  glEnableVertexArrayAttrib(mesh.vao, 3); // color => 3
  glVertexArrayAttribFormat(mesh.vao, 3, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
  glVertexArrayAttribBinding(mesh.vao, 3, 0);

  if (!mesh.vao || !mesh.vbo || !mesh.ebo)
  {
    std::cerr << "[LoadOBJ] Failed to create OBJ mesh\n";
    return Mesh{};
  }

  return mesh;
};


#endif // !VOXL_OBJ_LOADER_H