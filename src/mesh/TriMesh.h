///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/13/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.              //
///                                                                          //
/// Redistribution and use in source and binary forms, with or without       //
/// modification, are permitted provided that the following conditions are   //
/// met:                                                                     //
///  - Redistributions of source code must retain the above copyright        //
///    notice, this list of conditions and the following disclaimer.         //
///  - Redistributions in binary form must reproduce the above copyright     //
///    notice, this list of conditions and the following disclaimer in the   //
///    documentation and/or other materials provided with the distribution.  //
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  //
/// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    //
/// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          //
/// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       //
/// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         //
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    //
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    //
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      //
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    //
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     //
///                                                                          //
///--------------------------------------------------------------------------//

#ifndef QARAY_TRIMESH_H
#define QARAY_TRIMESH_H
#pragma once

#include "math/math.h"
#include <tiny_obj_loader.h>
#include <utility>
#include <cassert>
#include <array>

namespace qaray {
class TriMesh {
 public:
  struct TriFace {
    tinyobj::shape_t* shape = nullptr;
    tinyobj::index_t* v[3];
    size_t idx;
    int mtl;
    TriFace(tinyobj::shape_t& s,
            tinyobj::index_t& vt0,
            tinyobj::index_t& vt1,
            tinyobj::index_t& vt2,
            int m, size_t id)
        :
        shape(&s), mtl(m), v{&vt0, &vt1, &vt2}, idx(id)
    {}
    TriFace(const TriFace& t)
        :
        shape(t.shape), mtl(t.mtl), v{t.v[0], t.v[1], t.v[2]}, idx(t.idx)
    {}
    TriFace& operator = (const TriFace& t) {
      if (&t == this) { return *this; }
      shape = t.shape;
      mtl = t.mtl;
      v[0] = t.v[0];
      v[1] = t.v[1];
      v[2] = t.v[2];
      idx = t.idx;
    };
  };
 private:
  std::string path, name, file;
  tinyobj::attrib_t                attrib;
  std::vector<tinyobj::shape_t>    shapes;
  std::vector<tinyobj::material_t> materials;
  std::vector<TriFace> faces;
  std::vector<size_t> mcfc;
  vec3f boundMin;    //!< Bounding box minimum bound
  vec3f boundMax;    //!< Bounding box maximum bound
 public:

  //!@name Component Access Methods
  std::string GetDirectoryName() { return path; }
  std::string GetFileName() { return name; }
  std::string GetFullPath() { return file; }

  //!< returns the i^th face
  const TriFace& F(size_t i) const { return faces[i]; }
  TriFace& F(size_t i) { return faces[i]; }

  //!< returns the i^th vertex
  const vec3f &V(int i) const {
    return (const vec3f&)attrib.vertices[3 * i];
  }
  //!< returns the i^th vertex
  vec3f &V(int i) {
    return (vec3f&)attrib.vertices[3 * i];
  }

  //!< returns the i^th vertex normal
  const vec3f &VN(int i) const {
    return (const vec3f&)attrib.normals[3 * i];
  }
  //!< returns the i^th vertex normal
  vec3f &VN(int i) {
    return (vec3f&)attrib.normals[3 * i];
  }

  //!< returns the i^th vertex texture
  const vec2f &VT(int i) const {
    return (const vec2f&)attrib.texcoords[2 * i];
  }
  //!< returns the i^th vertex texture
  vec2f &VT(int i) { return (vec2f&)attrib.texcoords[2 * i]; }

  //!< returns the i^th material
  const tinyobj::material_t &M(int i) const { return materials[i]; }
  //!< returns the i^th material
  tinyobj::material_t &M(int i) { return materials[i]; }

  //!< returns the number of faces
  size_t NF() const { return faces.size(); }
  //!< returns the number of vertices
  size_t NV() const { return attrib.vertices.size() / 3; }
  //!< returns the number of vertex normals
  size_t NVN() const { return attrib.normals.size() / 3; }
  //!< returns the number of texture vertices
  size_t NVT() const { return attrib.texcoords.size() / 2; }
  //!< returns the number of materials
  size_t NM() const { return materials.size(); }

  //!< returns true if the mesh has vertex
  bool HasVertices(size_t faceID) const
  {
    return
        (faces[faceID].v[0]->vertex_index >= 0) &&
        (faces[faceID].v[1]->vertex_index >= 0) &&
        (faces[faceID].v[2]->vertex_index >= 0);
  }
  //!< returns true if the mesh has vertex normals
  bool HasNormals(size_t faceID) const
  {
    return
        (faces[faceID].v[0]->normal_index >= 0) &&
        (faces[faceID].v[1]->normal_index >= 0) &&
        (faces[faceID].v[2]->normal_index >= 0);
  }
  //!< returns true if the mesh has texture vertices
  bool HasTextureVertices(size_t faceID) const
  {
    return
        (faces[faceID].v[0]->texcoord_index >= 0) &&
        (faces[faceID].v[1]->texcoord_index >= 0) &&
        (faces[faceID].v[2]->texcoord_index >= 0);
  }

  //!@name Set Component Count
  //!< Deletes all components of the mesh
  void Clear()
  {
    attrib.vertices.clear();
    attrib.normals.clear();
    attrib.texcoords.clear();
    attrib.colors.clear();
    shapes.clear();
    materials.clear();
    faces.clear();
    boundMin = vec3f(1, 1, 1);
    boundMax = vec3f(0, 0, 0);
  }

  //!< Copies mesh data from the given mesh.
  TriMesh& operator=(const TriMesh &t) = default;

  //!@name Get Property Methods
  //!< Returns true if the bounding box has been computed.
  bool IsBoundBoxReady() const
  {
    return
        boundMin.x <= boundMax.x &&
        boundMin.y <= boundMax.y &&
        boundMin.z <= boundMax.z;
  }
  vec3f GetBoundMin() const { return boundMin; }        //!< Returns the minimum values of the bounding box
  vec3f GetBoundMax() const { return boundMax; }        //!< Returns the maximum values of the bounding box

  //!< Returns the point on the given face with the given barycentric coordinates (bc).
  vec3f GetPoint(size_t faceID, const vec3f &bc) const
  {
    assert(HasVertices(faceID));
    int v0 = faces[faceID].v[0]->vertex_index;
    int v1 = faces[faceID].v[1]->vertex_index;
    int v2 = faces[faceID].v[2]->vertex_index;
    return V(v0) * bc.x + V(v1) * bc.y + V(v2) * bc.z;
  }

  //!< Returns the the surface normal on the given face at the given barycentric
  //!< coordinates (bc). The returned vector is not normalized.
  vec3f GetNormal(size_t faceID, const vec3f &bc) const
  {
    assert(HasVertices(faceID));
    int v0 = faces[faceID].v[0]->normal_index;
    int v1 = faces[faceID].v[1]->normal_index;
    int v2 = faces[faceID].v[2]->normal_index;
    return VN(v0) * bc.x + VN(v1) * bc.y + VN(v2) * bc.z;
  }

  //!< Returns the texture coordinate on the given face at the given barycentric
  //!< coordinates (bc).
  vec2f GetTexCoord(size_t faceID, const vec3f &bc) const
  {
    assert(HasVertices(faceID));
    int v0 = faces[faceID].v[0]->texcoord_index;
    int v1 = faces[faceID].v[1]->texcoord_index;
    int v2 = faces[faceID].v[2]->texcoord_index;
    return VT(v0) * bc.x + VT(v1) * bc.y + VT(v2) * bc.z;
  }
  //!< Returns the material index of the face. This method goes through material
  //!< counts of all materials to find the material index of the face. Returns a
  //!< negative number if the face as no material
  int GetMaterialIndex(size_t faceID) const
  {
    return faces[faceID].mtl;
  }
  //!< Returns the number of faces associated with the given material ID.
  size_t GetMaterialFaceCount(int mtlID) const;
  //!< Returns the first face index associated with the given material ID. Other faces associated with the same material are placed are placed consecutively.
  size_t GetMaterialFirstFace(int mtlID) const;

  //!@name Compute Methods
  void ComputeBoundingBox();                   //!< Computes the bounding box
  void ComputeNormals(bool clockwise = false); //!< Computes and stores vertex normals

  //!@name Load and Save methods
  bool LoadFromFileObj(const char *filename,
                       bool loadMtl = true,
                       std::ostream *outStream = &std::cout);    //!< Loads the mesh from an OBJ file. Automatically converts all faces to triangles..
};
}

#endif //QARAY_TRIMESH_H
