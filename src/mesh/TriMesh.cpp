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

#define TINYOBJLOADER_IMPLEMENTATION
#include "TriMesh.h"

//----------------------------------------------------------------------------
static std::string ParsePath(const std::string &str)
{
  std::string cstr = str;
#if                        \
  defined(WIN32) || \
  defined(_WIN32) || \
  defined(__WIN32) && \
  !defined(__CYGWIN__)
  std::replace(cstr.begin(), cstr.end(), '/', '\\');
#else
  std::replace(cstr.begin(), cstr.end(), '\\', '/');
#endif
  return cstr;
}
//----------------------------------------------------------------------------
static std::string ComputePath(const std::string &str, std::string &dpath, std::string& fname)
{
  std::string fpath = ParsePath(str);
  size_t p = fpath.find_last_of("/\\");
  if (p != std::string::npos) {
    dpath = fpath.substr(0, p + 1);
    fname = fpath.substr(p + 1, fpath.size() - dpath.size());
  } else {
    dpath = "";
    fname = fpath;
  }
  return fpath;
}
//----------------------------------------------------------------------------

namespace qaray {
bool TriMesh::LoadFromFileObj(const char *filename,
                              bool loadMtl,
                              std::ostream *outStream)
{
  //
  // Read OBJ file using tinyobjloader
  //
  // reference https://github.com/syoyo/tinyobjloader
  //
  // load file
  file = ComputePath(filename, path, name);
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                       file.c_str(), path.c_str(), true);
  // err may contain warning message.
  if (!err.empty()) { *outStream << std::endl << err << std::endl; }
  if (!ret) { return false; }
  // post processing
  mcfc.resize(materials.size(), 0);
  // Loop over shapes
  for (auto& shape : shapes) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
      int fv = shape.mesh.num_face_vertices[f];
      if (fv != 3) {
        *outStream << std::endl << "Error: This mesh is not a triangular mesh."
                   << std::endl;
        return false;
      }
      // Loop over vertices in the face.
      int mtl_id = shape.mesh.material_ids[f];
      faces.emplace_back(shape,
                         shape.mesh.indices[index_offset + 0],
                         shape.mesh.indices[index_offset + 1],
                         shape.mesh.indices[index_offset + 2],
                         mtl_id);
      if (mtl_id >= 0) {
        ++mcfc[mtl_id];
      }
      index_offset += fv;
    }
  }

  std::sort(faces.begin(), faces.end(), [](const TriFace& a, const TriFace& b) {
    if (a.mtl >= 0 && b.mtl >= 0) {
      return a.mtl < b.mtl;
    }
    else {
      return false;
    }
  });
  return true;
}
void TriMesh::ComputeBoundingBox() {
  if (NV() > 0) {
    boundMin = V(0);
    boundMax = V(0);
    for (unsigned int i = 1; i < NV(); i++) {
      if (boundMin.x > V(i).x) boundMin.x = V(i).x;
      if (boundMin.y > V(i).y) boundMin.y = V(i).y;
      if (boundMin.z > V(i).z) boundMin.z = V(i).z;
      if (boundMax.x < V(i).x) boundMax.x = V(i).x;
      if (boundMax.y < V(i).y) boundMax.y = V(i).y;
      if (boundMax.z < V(i).z) boundMax.z = V(i).z;
    }
  } else {
    boundMin = vec3f(1, 1, 1);
    boundMax = vec3f(0, 0, 0);
  }
}
void TriMesh::ComputeNormals(bool clockwise) {
  attrib.normals.clear();
  attrib.normals.resize(NV());
  for (unsigned int i = 0; i < NF(); i++) {
    // initialize all normals to zero
    VN(i) = vec3f(0, 0, 0);
  }
  for (unsigned int i = 0; i < NF(); i++) {
    // face normal (not normalized)
    vec3f N = qaray::cross((V(faces[i].v[1]->vertex_index) -
                               V(faces[i].v[0]->vertex_index)),
                           (V(faces[i].v[2]->vertex_index) -
                               V(faces[i].v[0]->vertex_index)));
    if (clockwise) N = -N;
    VN(faces[i].v[0]->vertex_index) += N;
    VN(faces[i].v[1]->vertex_index) += N;
    VN(faces[i].v[2]->vertex_index) += N;
    faces[i].v[0]->normal_index = faces[i].v[0]->vertex_index;
    faces[i].v[1]->normal_index = faces[i].v[1]->vertex_index;
    faces[i].v[2]->normal_index = faces[i].v[2]->vertex_index;
  }
  for (unsigned int i = 0; i < NF(); i++) {
    VN(i) = normalize(VN(i));
  }
}

//!< Returns the number of faces associated with the given material ID.
size_t TriMesh::GetMaterialFaceCount(int mtlID) const
{
  return mtlID > 0 ? mcfc[mtlID] - mcfc[mtlID - 1] : mcfc[0];
}

//!< Returns the first face index associated with the given material ID. Other faces associated with the same material are placed are placed consecutively.
size_t TriMesh::GetMaterialFirstFace(int mtlID) const
{
  return mtlID > 0 ? mcfc[mtlID - 1] : 0;
}

}