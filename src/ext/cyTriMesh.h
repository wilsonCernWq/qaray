// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
//! \file   cyTriMesh.h 
//! \author Cem Yuksel
//! 
//! \brief  Triangular Mesh class.
//! 
//-------------------------------------------------------------------------------
//
// Copyright (c) 2016, Cem Yuksel <cem@cemyuksel.com>
// All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the "Software"), to deal 
// in the Software without restriction, including without limitation the rights 
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
// 
//-------------------------------------------------------------------------------

#ifndef _CY_TRIMESH_H_INCLUDED_
#define _CY_TRIMESH_H_INCLUDED_

//-------------------------------------------------------------------------------

#include "math/math.h"
#include <tiny_obj_loader.h>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <iostream>

//-------------------------------------------------------------------------------
namespace cy {
//-------------------------------------------------------------------------------

//! Triangular Mesh Class

class TriMesh {
 private:
  using Point3f = qaray::Point3;
  //----------------------------------------------------------------------------
  std::string ParsePath(const std::string &str)
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
  void
  ComputePath(const std::string &str, std::string &dpath)
  {
    std::string fname; // dummy
    std::string fpath = ParsePath(str);
    size_t p = fpath.find_last_of("/\\");
    if (p != std::string::npos) {
      dpath = fpath.substr(0, p + 1);
      fname = fpath.substr(p + 1, fpath.size() - dpath.size());
    } else {
      dpath = "";
      fname = fpath;
    }
  }
 public:
  //! Triangular Mesh Face
  struct TriFace {
    unsigned int v[3];    //!< vertex indices
  };

  //! Simple character string
  struct Str {
    char *data;    //!< String data
    Str() : data(nullptr) {}                            //!< Constructor
    Str(const Str &s) : data(nullptr) { *this = s; }    //!< Copy constructor
    ~Str() { if (data) delete[] data; }                //!< Destructor
    operator const char *() { return data; }            //!< Implicit conversion to const char
    void operator=(const Str &s) { *this = s.data; }    //!< Assignment operator
    void operator=(const char *s)
    {
      if (s) {
        size_t n = strlen(s);
        if (data) delete[] data;
        data = new char[n + 1];
        strncpy(data, s, n);
        data[n] = '\0';
      } else if (data) {
        delete[] data;
        data = nullptr;
      }
    }    //!< Assignment operator
  };

  //! Material definition
  struct Mtl {
    Str name;        //!< Material name
    float Ka[3];    //!< Ambient color
    float Kd[3];    //!< Diffuse color
    float Ks[3];    //!< Specular color
    float Tf[3];    //!< Transmission color
    float Ns;        //!< Specular exponent
    float Ni;        //!< Index of refraction
    int illum;    //!< Illumination model
    Str map_Ka;    //!< Ambient color texture map
    Str map_Kd;    //!< Diffuse color texture map
    Str map_Ks;    //!< Specular color texture map
    Str map_Ns;    //!< Specular exponent texture map
    Str map_d;    //!< Alpha texture map
    Str map_bump;    //!< Bump texture map
    Str map_disp;    //!< Displacement texture map

    //! Constructor sets the default material values
    Mtl()
    {
      Ka[0] = Ka[1] = Ka[2] = 0;
      Kd[0] = Kd[1] = Kd[2] = 1;
      Ks[0] = Ks[1] = Ks[2] = 0;
      Tf[0] = Tf[1] = Tf[2] = 0;
      Ns = 0;
      Ni = 1;
      illum = 2;
    }
  };

  std::string GetDirectory() { return directory_name; }

 protected:
  std::string directory_name;
  Point3f *v;        //!< vertices
  TriFace *f;        //!< faces
  Point3f *vn;    //!< vertex normal
  TriFace *fn;    //!< normal faces
  Point3f *vt;    //!< texture vertices
  TriFace *ft;    //!< texture faces
  Mtl *m;        //!< materials
  int *mcfc;    //!< material cumulative face count

  unsigned int nv;    //!< number of vertices
  unsigned int nf;    //!< number of faces
  unsigned int nvn;    //!< number of vertex normals
  unsigned int nvt;    //!< number of texture vertices
  unsigned int nm;    //!< number of materials

  Point3f boundMin;    //!< Bounding box minimum bound
  Point3f boundMax;    //!< Bounding box maximum bound

 public:

  //!@name Constructors and Destructor
  TriMesh()
      : v(nullptr),
        f(nullptr),
        vn(nullptr),
        fn(nullptr),
        vt(nullptr),
        ft(nullptr),
        m(nullptr),
        mcfc(nullptr),
        nv(0),
        nf(0),
        nvn(0),
        nvt(0),
        nm(0),
        boundMin(1, 1, 1),
        boundMax(0, 0, 0) {}
  TriMesh(const TriMesh &t)
      : v(nullptr),
        f(nullptr),
        vn(nullptr),
        fn(nullptr),
        vt(nullptr),
        ft(nullptr),
        m(nullptr),
        mcfc(nullptr),
        nv(0),
        nf(0),
        nvn(0),
        nvt(0),
        nm(0),
        boundMin(1, 1, 1),
        boundMax(0, 0, 0) { *this = t; }
  virtual ~TriMesh() { Clear(); }

  //!@name Component Access Methods
  const Point3f &V(int i) const { return v[i]; }        //!< returns the i^th vertex
  Point3f &V(int i) { return v[i]; }        //!< returns the i^th vertex
  const TriFace &F(int i) const { return f[i]; }        //!< returns the i^th face
  TriFace &F(int i) { return f[i]; }        //!< returns the i^th face
  const Point3f &VN(int i) const { return vn[i]; }    //!< returns the i^th vertex normal
  Point3f &VN(int i) { return vn[i]; }    //!< returns the i^th vertex normal
  const TriFace &FN(int i) const { return fn[i]; }    //!< returns the i^th normal face
  TriFace &FN(int i) { return fn[i]; }    //!< returns the i^th normal face
  const Point3f &VT(int i) const { return vt[i]; }    //!< returns the i^th vertex texture
  Point3f &VT(int i) { return vt[i]; }    //!< returns the i^th vertex texture
  const TriFace &FT(int i) const { return ft[i]; }    //!< returns the i^th texture face
  TriFace &FT(int i) { return ft[i]; }    //!< returns the i^th texture face
  const Mtl &M(int i) const { return m[i]; }        //!< returns the i^th material
  Mtl &M(int i) { return m[i]; }        //!< returns the i^th material

  unsigned int NV() const { return nv; }        //!< returns the number of vertices
  unsigned int NF() const { return nf; }        //!< returns the number of faces
  unsigned int NVN() const { return nvn; }    //!< returns the number of vertex normals
  unsigned int NVT() const { return nvt; }    //!< returns the number of texture vertices
  unsigned int NM() const { return nm; }        //!< returns the number of materials

  bool HasNormals() const
  {
    return NVN() > 0;
  }            //!< returns true if the mesh has vertex normals
  bool HasTextureVertices() const
  {
    return NVT() > 0;
  }    //!< returns true if the mesh has texture vertices

  //!@name Set Component Count
  void Clear()
  {
    SetNumVertex(0);
    SetNumFaces(0);
    SetNumNormals(0);
    SetNumTexVerts(0);
    SetNumMtls(0);
    boundMin = Point3f(1, 1, 1);
    boundMax = Point3f(0, 0, 0);
  }    //!< Deletes all components of the mesh
  void SetNumVertex(unsigned int n)
  {
    Allocate(n,
             v,
             nv);
  }                                                            //!< Sets the number of vertices and allocates memory for vertex positions
  void SetNumFaces(unsigned int n)
  {
    Allocate(n, f, nf);
    if (fn || vn) Allocate(n, fn);
    if (ft || vt) Allocate(n, ft);
  }    //!< Sets the number of faces and allocates memory for face data. Normal faces and texture faces are also allocated, if they are used.
  void SetNumNormals(unsigned int n)
  {
    Allocate(n, vn, nvn);
    Allocate(n == 0 ? 0 : nf, fn);
  }                                    //!< Sets the number of normals and allocates memory for normals and normal faces.
  void SetNumTexVerts(unsigned int n)
  {
    Allocate(n, vt, nvt);
    Allocate(n == 0 ? 0 : nf, ft);
  }                                    //!< Sets the number of texture coordinates and allocates memory for texture coordinates and texture faces.
  void SetNumMtls(unsigned int n)
  {
    Allocate(n, m, nm);
    Allocate(n, mcfc);
  }                                            //!< Sets the number of materials and allocates memory for material data.
  void operator=(const TriMesh &t);                                                                                    //!< Copies mesh data from the given mesh.

  //!@name Get Property Methods
  bool IsBoundBoxReady() const
  {
    return boundMin.x <= boundMax.x && boundMin.y <= boundMax.y
        && boundMin.z <= boundMax.z;
  }    //!< Returns true if the bounding box has been computed.
  Point3f GetBoundMin() const { return boundMin; }        //!< Returns the minimum values of the bounding box
  Point3f GetBoundMax() const { return boundMax; }        //!< Returns the maximum values of the bounding box
  Point3f GetPoint(int faceID, const Point3f &bc) const
  {
    return Interpolate(faceID,
                       v,
                       f,
                       bc);
  }        //!< Returns the point on the given face with the given barycentric coordinates (bc).
  Point3f GetNormal(int faceID, const Point3f &bc) const
  {
    return Interpolate(faceID,
                       vn,
                       fn,
                       bc);
  }    //!< Returns the the surface normal on the given face at the given barycentric coordinates (bc). The returned vector is not normalized.
  Point3f GetTexCoord(int faceID, const Point3f &bc) const
  {
    return Interpolate(faceID,
                       vt,
                       ft,
                       bc);
  }    //!< Returns the texture coordinate on the given face at the given barycentric coordinates (bc).
  int GetMaterialIndex(int faceID) const;                //!< Returns the material index of the face. This method goes through material counts of all materials to find the material index of the face. Returns a negative number if the face as no material
  int GetMaterialFaceCount(int mtlID) const
  {
    return mtlID > 0 ? mcfc[mtlID] - mcfc[mtlID - 1] : mcfc[0];
  }    //!< Returns the number of faces associated with the given material ID.
  int GetMaterialFirstFace(int mtlID) const
  {
    return mtlID > 0 ? mcfc[mtlID - 1] : 0;
  }    //!< Returns the first face index associated with the given material ID. Other faces associated with the same material are placed are placed consecutively.

  //!@name Compute Methods
  void ComputeBoundingBox();                        //!< Computes the bounding box
  void ComputeNormals(bool clockwise = false);        //!< Computes and stores vertex normals

  //!@name Load and Save methods
  bool LoadFromFileObj(const char *filename,
                       bool loadMtl = true,
                       std::ostream *outStream = &std::cout);    //!< Loads the mesh from an OBJ file. Automatically converts all faces to triangles.
  bool SaveToFileObj(const char *filename,
                     std::ostream *outStream);                                    //!< Saves the mesh to an OBJ file with the given name.

 private:
  template<class T>
  void Allocate(unsigned int n, T *&t)
  {
    if (t) delete[] t;
    if (n > 0) t = new T[n]; else t = nullptr;
  }
  template<class T>
  bool Allocate(unsigned int n, T *&t, unsigned int &nt)
  {
    if (n == nt)
      return false;
    nt = n;
    Allocate(n, t);
    return true;
  }
  template<class T>
  void Copy(const T *from, unsigned int n, T *&t, unsigned int &nt)
  {
    if (!from)
      n = 0;
    Allocate(n, t, nt);
    if (t) memcpy(t, from, sizeof(T) * n);
  }
  template<class T>
  void Copy(const T *from, unsigned int n, T *&t)
  {
    if (!from) n = 0;
    Allocate(n, t);
    if (t) memcpy(t, from, sizeof(T) * n);
  }
  static Point3f Interpolate(int i,
                             const Point3f *v,
                             const TriFace *f,
                             const Point3f &bc)
  {
    return v[f[i].v[0]] * bc.x + v[f[i].v[1]] * bc.y + v[f[i].v[2]] * bc.z;
  }

  // Temporary structures
  struct MtlData {
    std::string mtlName;
    unsigned int firstFace;
    unsigned int faceCount;
    MtlData()
    {
      faceCount = 0;
      firstFace = 0;
    }
  };
  struct MtlLibName { std::string filename; };
};

//-------------------------------------------------------------------------------

inline void TriMesh::operator=(const TriMesh &t)
{
  Copy(t.v, t.nv, v, nv);
  Copy(t.f, t.nf, f, nf);
  Copy(t.vn, t.nvn, vn, nvn);
  Copy(t.fn, t.nf, fn);
  Copy(t.vt, t.nvt, vt, nvt);
  Copy(t.ft, t.nf, ft);
  Allocate(t.nm, m, nm);
  for (unsigned int i = 0; i < nm; i++) m[i] = t.m[i];
  Copy(t.mcfc, t.nm, mcfc);
  boundMin = t.boundMin;
  boundMax = t.boundMax;
}

inline int TriMesh::GetMaterialIndex(int faceID) const
{
  for (unsigned int i = 0; i < nm; i++) {
    if (faceID < mcfc[i]) return (int) i;
  }
  return -1;
}

inline void TriMesh::ComputeBoundingBox()
{
  if (nv > 0) {
    boundMin = v[0];
    boundMax = v[0];
    for (unsigned int i = 1; i < nv; i++) {
      if (boundMin.x > v[i].x) boundMin.x = v[i].x;
      if (boundMin.y > v[i].y) boundMin.y = v[i].y;
      if (boundMin.z > v[i].z) boundMin.z = v[i].z;
      if (boundMax.x < v[i].x) boundMax.x = v[i].x;
      if (boundMax.y < v[i].y) boundMax.y = v[i].y;
      if (boundMax.z < v[i].z) boundMax.z = v[i].z;
    }
  } else {
    boundMin = Point3f(1, 1, 1);
    boundMax = Point3f(0, 0, 0);
  }
}

inline void TriMesh::ComputeNormals(bool clockwise)
{
  SetNumNormals(nv);
  for (unsigned int i = 0; i < nvn; i++)
    vn[i] = Point3f(0, 0, 0);    // initialize all normals to zero
  for (unsigned int i = 0; i < nf; i++) {
    Point3f N = qaray::cross((v[f[i].v[1]] - v[f[i].v[0]]),
                             (v[f[i].v[2]] - v[f[i]
                                 .v[0]]));    // face normal (not normalized)
    if (clockwise) N = -N;
    vn[f[i].v[0]] += N;
    vn[f[i].v[1]] += N;
    vn[f[i].v[2]] += N;
    fn[i] = f[i];
  }
  for (unsigned int i = 0; i < nvn; i++) { vn[i] = normalize(vn[i]); }
}

inline bool TriMesh::LoadFromFileObj(const char *filename,
                                     bool loadMtl,
                                     std::ostream *outStream)
{
#ifdef TINY_OBJ_LOADER_H_
  //
  // Read OBJ file using tinyobjloader
  //
  /* reference https://github.com/syoyo/tinyobjloader */
  std::string inputfile(filename);
  TriMesh::ComputePath(filename, directory_name);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                       inputfile.c_str(), directory_name.c_str());
  // `err` may contain warning message.
  if (!err.empty()) { *outStream << err << std::endl; }
  if (!ret) { return false; }
  //
  // Setup fields in class
  //
  Clear();
  //-------------------------------------------------------------------------//
  struct MtlList {
    std::vector<MtlData> mtlData;
    int GetMtlIndex(const char *mtlName)
    {
      for (unsigned int i = 0; i < mtlData.size(); i++) {
        if (mtlData[i].mtlName == mtlName) return (int) i;
      }
      return -1;
    }
    int CreateMtl(const char *mtlName, unsigned int firstFace)
    {
      if (mtlName[0] == '\0') return 0;
      int i = GetMtlIndex(mtlName);
      if (i >= 0) return i;
      MtlData m;
      m.mtlName = mtlName;
      m.firstFace = firstFace;
      mtlData.push_back(m);
      return (int) mtlData.size() - 1;
    }
  };
  MtlList mtlList;
  if (loadMtl) {
    for (size_t _i = 0; _i < materials.size(); ++_i) {
      // create new material
      auto &_mtl = materials[_i];
      mtlList.CreateMtl(_mtl.name.c_str(), -1); // will add face later
    }
  }
  //-------------------------------------------------------------------------//
  std::vector<Point3f> _v;     // vertices
  std::vector<TriFace> _f;     // faces
  std::vector<Point3f> _vn;    // vertex normal
  std::vector<TriFace> _fn;    // normal faces
  std::vector<Point3f> _vt;    // texture vertices
  std::vector<TriFace> _ft;    // texture faces
  std::vector<int> faceMtlIndex;
  //-------------------------------------------------------------------------//
  //-------------------------------------------------------------------------//
  // Loop over shapes
  _v.resize(attrib.vertices.size() / 3);
  std::copy(attrib.vertices.begin(),
            attrib.vertices.end(),
            (float *) _v.data());
  _vn.resize(attrib.normals.size() / 3);
  std::copy(attrib.normals.begin(), attrib.normals.end(), (float *) _vn.data());
  _vt.resize(attrib.texcoords.size() / 2);
  for (size_t i = 0; i < attrib.texcoords.size() / 2; ++i) {
    _vt[i] = Point3f(attrib.texcoords[2 * i + 0],
                     attrib.texcoords[2 * i + 1],
                     0.f);
  }

  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      int fv = shapes[s].mesh.num_face_vertices[f];
      if (fv != 3) {
        *outStream << "Error: This mesh is not a triangular mesh."
                   << std::endl;
        return false;
      }
      // Process faces
      TriFace face, normalFace, textureFace;
      bool hasNormals = false, hasTextures = false;
      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) { // fv == 3
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        // Vertex
        //tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
        //tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
        //tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
        face.v[v] = static_cast<unsigned int>(idx.vertex_index);
        // Normal
        if (idx.normal_index >= 0) { // -1 as invalid
          //tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
          //tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
          //tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
          normalFace.v[v] = static_cast<unsigned int>(idx.normal_index);
          hasNormals = true;
        }
        // Texture
        if (idx.texcoord_index >= 0) { // -1 as invalid
          // tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
          // tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
          // _vt.emplace_back(tx, ty, 0.f);
          textureFace.v[v] = static_cast<unsigned int>(idx.texcoord_index);
          hasTextures = true;
        }
        // Optional: vertex colors
        // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
        // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
        // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
      }
      // Per-face material
      auto _mtl_idx = shapes[s].mesh.material_ids[f];
      faceMtlIndex.push_back(_mtl_idx);
      if (_mtl_idx >= 0 && loadMtl) { // -1 as invalid
        if (mtlList.mtlData[_mtl_idx].faceCount == 0) {
          mtlList.mtlData[_mtl_idx].firstFace =
              static_cast<unsigned int>(_f.size());
        }
        ++mtlList.mtlData[_mtl_idx].faceCount;
      }
      // Process faces
      _f.push_back(face);
      if (hasTextures) _ft.push_back(textureFace);
      if (hasNormals) _fn.push_back(normalFace);
      // increment
      index_offset += fv;
    }
  }

  // Setup fields
  if (_f.empty()) return true; // No faces found
  SetNumVertex((unsigned int) _v.size());
  SetNumFaces((unsigned int) _f.size());
  SetNumTexVerts((unsigned int) _vt.size());
  SetNumNormals((unsigned int) _vn.size());
  if (loadMtl) SetNumMtls((unsigned int) mtlList.mtlData.size());

  // Copy data
  memcpy(v, _v.data(), sizeof(Point3f) * _v.size());
  if (!_vt.empty()) memcpy(vt, _vt.data(), sizeof(Point3f) * _vt.size());
  if (!_vn.empty()) memcpy(vn, _vn.data(), sizeof(Point3f) * _vn.size());

  // Post-process materials face count
  if (!mtlList.mtlData.empty()) {
    unsigned int fid = 0;
    for (int m = 0; m < (int) mtlList.mtlData.size(); m++) {
      for (unsigned int i = mtlList.mtlData[m].firstFace, j = 0;
           j < mtlList.mtlData[m].faceCount && i < _f.size(); i++) {
        if (faceMtlIndex[i] == m) {
          f[fid] = _f[i];
          if (fn) fn[fid] = _fn[i];
          if (ft) ft[fid] = _ft[i];
          fid++;
          j++;
        }
      }
      mcfc[m] = fid;
    }
    if (fid < _f.size()) {
      for (unsigned int i = 0; i < _f.size(); i++) {
        if (faceMtlIndex[i] < 0) {
          f[fid] = _f[i];
          if (fn) fn[fid] = _fn[i];
          if (ft) ft[fid] = _ft[i];
          fid++;
        }
      }
    }
  } else {
    memcpy(f, _f.data(), sizeof(TriFace) * _f.size());
    if (ft) memcpy(ft, _ft.data(), sizeof(TriFace) * _ft.size());
    if (fn) memcpy(fn, _fn.data(), sizeof(TriFace) * _fn.size());
  }

  // Process material values
  if (loadMtl) {
    for (size_t _i = 0; _i < materials.size(); ++_i) {
      // create new material
      auto &_mtl = materials[_i];
      // ambient Ka
      m[_i].Ka[0] = _mtl.ambient[0];
      m[_i].Ka[1] = _mtl.ambient[1];
      m[_i].Ka[2] = _mtl.ambient[2];
      // diffuse Kd
      m[_i].Kd[0] = _mtl.diffuse[0];
      m[_i].Kd[1] = _mtl.diffuse[1];
      m[_i].Kd[2] = _mtl.diffuse[2];
      // specular Ks
      m[_i].Ks[0] = _mtl.specular[0];
      m[_i].Ks[1] = _mtl.specular[1];
      m[_i].Ks[2] = _mtl.specular[2];
      // transmittance Tf || Kt
      m[_i].Tf[0] = _mtl.transmittance[0];
      m[_i].Tf[1] = _mtl.transmittance[1];
      m[_i].Tf[2] = _mtl.transmittance[2];
      // shininess Ns
      m[_i].Ns = _mtl.shininess;
      // ior Ni
      m[_i].Ni = _mtl.ior;
      // illum
      m[_i].illum = _mtl.illum;
      // ambient_texname -- map_Ka
      // diffuse_texname -- map_Kd
      // specular_texname -- mal_Ks
      m[_i].map_Ka =
          _mtl.ambient_texname.empty() ? nullptr : _mtl.ambient_texname.c_str();
      m[_i].map_Kd =
          _mtl.diffuse_texname.empty() ? nullptr : _mtl.diffuse_texname.c_str();
      m[_i].map_Ks =
          _mtl.specular_texname.empty() ? nullptr : _mtl.specular_texname
              .c_str();
      // specular_highlight_texname -- map_Ns
      m[_i].map_Ns = _mtl.specular_highlight_texname.empty() ? nullptr : _mtl
          .specular_highlight_texname.c_str();
      // alpha_texname -- map_d
      m[_i].map_d =
          _mtl.alpha_texname.empty() ? nullptr : _mtl.alpha_texname.c_str();
      // bump_texname bump -- map_bump
      m[_i].map_bump =
          _mtl.bump_texname.empty() ? nullptr : _mtl.bump_texname.c_str();
      // displacement_texname -- map_disp
      m[_i].map_disp = _mtl.displacement_texname.empty() ? nullptr : _mtl
          .displacement_texname.c_str();
    }
  }

#else

  ComputePath(filename, directory_name);

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    if (outStream)
      *outStream << "ERROR: Cannot open file " << filename << std::endl;
    return false;
  }

  Clear();

  class Buffer {
    char data[1024];
    int readLine;
   public:
    int ReadLine(FILE *fp)
    {
      char c = fgetc(fp);
      while (!feof(fp)) {
        while (isspace(c) && (!feof(fp) || c != '\0'))
          c = fgetc(fp);    // skip empty space
        if (c == '#')
          while (!feof(fp) && c != '\n' && c != '\r' && c != '\0')
            c = fgetc(fp);    // skip comment line
        else break;
      }
      int i = 0;
      bool inspace = false;
      while (i < 1024 - 1) {
        if (feof(fp) || c == '\n' || c == '\r' || c == '\0') break;
        if (isspace(c)) {    // only use a single space as the space character
          inspace = true;
        } else {
          if (inspace) data[i++] = ' ';
          inspace = false;
          data[i++] = c;
        }
        c = fgetc(fp);
      }
      data[i] = '\0';
      readLine = i;
      return i;
    }
    char &operator[](int i) { return data[i]; }
    void ReadVertex(Point3f &v) const
    {
      v = Point3f(0, 0, 0);
      sscanf(data + 2, "%f %f %f", &v.x, &v.y, &v.z);
    }
    void ReadFloat3(float f[3]) const
    {
      f[2] = f[1] = f[0] = 0;
      int n = sscanf(data + 2, "%f %f %f", &f[0], &f[1], &f[2]);
      if (n == 1) f[2] = f[1] = f[0];
    }
    void ReadFloat(float *f) const { sscanf(data + 2, "%f", f); }
    void ReadInt(int *i, int start) const { sscanf(data + start, "%d", i); }
    bool IsCommand(const char *cmd) const
    {
      int i = 0;
      while (cmd[i] != '\0') {
        if (cmd[i] != data[i]) return false;
        i++;
      }
      return (data[i] == '\0' || data[i] == ' ');
    }
    const char *Data(int start = 0) { return data + start; }
    void Copy(Str &str, int start = 0)
    {
      while (data[start] != '\0' && data[start] <= ' ') start++;
      str = Data(start);
    }
  };
  Buffer buffer;

  struct MtlList {
    std::vector<MtlData> mtlData;
    int GetMtlIndex(const char *mtlName)
    {
      for (unsigned int i = 0; i < mtlData.size(); i++) {
        if (mtlData[i].mtlName == mtlName) return (int) i;
      }
      return -1;
    }
    int CreateMtl(const char *mtlName, unsigned int firstFace)
    {
      if (mtlName[0] == '\0') return 0;
      int i = GetMtlIndex(mtlName);
      if (i >= 0) return i;
      MtlData m;
      m.mtlName = mtlName;
      m.firstFace = firstFace;
      mtlData.push_back(m);
      return (int) mtlData.size() - 1;
    }
  };
  MtlList mtlList;

  std::vector<Point3f> _v;        // vertices
  std::vector<TriFace> _f;        // faces
  std::vector<Point3f> _vn;    // vertex normal
  std::vector<TriFace> _fn;    // normal faces
  std::vector<Point3f> _vt;    // texture vertices
  std::vector<TriFace> _ft;    // texture faces
  std::vector<MtlLibName> mtlFiles;
  std::vector<int> faceMtlIndex;

  int currentMtlIndex = -1;
  bool hasTextures = false, hasNormals = false;

  while (int rb = buffer.ReadLine(fp)) {
    if (buffer.IsCommand("v")) {
      Point3f vertex;
      buffer.ReadVertex(vertex);
      _v.push_back(vertex);
    } else if (buffer.IsCommand("vt")) {
      Point3f texVert;
      buffer.ReadVertex(texVert);
      _vt.push_back(texVert);
      hasTextures = true;
    } else if (buffer.IsCommand("vn")) {
      Point3f normal;
      buffer.ReadVertex(normal);
      _vn.push_back(normal);
      hasNormals = true;
    } else if (buffer.IsCommand("f")) {
      int facevert = -1;
      bool inspace = true;
      bool negative = false;
      int type = 0;
      unsigned int index;
      TriFace face, textureFace, normalFace;
      unsigned int nFacesBefore = (unsigned int) _f.size();
      for (int i = 2; i < rb; i++) {
        if (buffer[i] == ' ') inspace = true;
        else {
          if (inspace) {
            inspace = false;
            negative = false;
            type = 0;
            index = 0;
            switch (facevert) {
              case -1:
                // initialize face
                face.v[0] = face.v[1] = face.v[2] = 0;
                textureFace.v[0] = textureFace.v[1] = textureFace.v[2] = 0;
                normalFace.v[0] = normalFace.v[1] = normalFace.v[2] = 0;
              case 0:
              case 1: facevert++;
                break;
              case 2:
                // copy the first two vertices from the previous face
                _f.push_back(face);
                face.v[1] = face.v[2];
                if (hasTextures) {
                  _ft.push_back(textureFace);
                  textureFace.v[1] = textureFace.v[2];
                }
                if (hasNormals) {
                  _fn.push_back(normalFace);
                  normalFace.v[1] = normalFace.v[2];
                }
                faceMtlIndex.push_back(currentMtlIndex);
                break;
            }
          }
          if (buffer[i] == '/') {
            type++;
            index = 0;
          }
          if (buffer[i] == '-') negative = true;
          if (buffer[i] >= '0' && buffer[i] <= '9') {
            index = index * 10 + (buffer[i] - '0');
            switch (type) {
              case 0:
                face.v[facevert] =
                    negative ? (unsigned int) _v.size() - index : index - 1;
                break;
              case 1:
                textureFace.v[facevert] =
                    negative ? (unsigned int) _vt.size() - index : index - 1;
                hasTextures = true;
                break;
              case 2:
                normalFace.v[facevert] =
                    negative ? (unsigned int) _vn.size() - index : index - 1;
                hasNormals = true;
                break;
            }
          }
        }
      }
      _f.push_back(face);
      if (hasTextures) _ft.push_back(textureFace);
      if (hasNormals) _fn.push_back(normalFace);
      faceMtlIndex.push_back(currentMtlIndex);
      if (currentMtlIndex >= 0)
        mtlList.mtlData[currentMtlIndex].faceCount +=
            (unsigned int) _f.size() - nFacesBefore;
    } else if (loadMtl) {
      if (buffer.IsCommand("usemtl")) {
        currentMtlIndex =
            mtlList.CreateMtl(buffer.Data(7), (unsigned int) _f.size());
      }
      if (buffer.IsCommand("mtllib")) {
        MtlLibName libName;
        libName.filename = buffer.Data(7);
        mtlFiles.push_back(libName);
      }
    }
    if (feof(fp)) break;
  }

  fclose(fp);

  if (_f.size() == 0) return true; // No faces found
  SetNumVertex((unsigned int) _v.size());
  SetNumFaces((unsigned int) _f.size());
  SetNumTexVerts((unsigned int) _vt.size());
  SetNumNormals((unsigned int) _vn.size());
  if (loadMtl) SetNumMtls((unsigned int) mtlList.mtlData.size());

  // Copy data
  memcpy(v, _v.data(), sizeof(Point3f) * _v.size());
  if (_vt.size() > 0) memcpy(vt, _vt.data(), sizeof(Point3f) * _vt.size());
  if (_vn.size() > 0) memcpy(vn, _vn.data(), sizeof(Point3f) * _vn.size());

  if (mtlList.mtlData.size() > 0) {
    unsigned int fid = 0;
    for (int m = 0; m < (int) mtlList.mtlData.size(); m++) {
      for (unsigned int i = mtlList.mtlData[m].firstFace, j = 0;
           j < mtlList.mtlData[m].faceCount && i < _f.size(); i++) {
        if (faceMtlIndex[i] == m) {
          f[fid] = _f[i];
          if (fn) fn[fid] = _fn[i];
          if (ft) ft[fid] = _ft[i];
          fid++;
          j++;
        }
      }
      mcfc[m] = fid;
    }
    if (fid < _f.size()) {
      for (unsigned int i = 0; i < _f.size(); i++) {
        if (faceMtlIndex[i] < 0) {
          f[fid] = _f[i];
          if (fn) fn[fid] = _fn[i];
          if (ft) ft[fid] = _ft[i];
          fid++;
        }
      }
    }
  } else {
    memcpy(f, _f.data(), sizeof(TriFace) * _f.size());
    if (ft) memcpy(ft, _ft.data(), sizeof(TriFace) * _ft.size());
    if (fn) memcpy(fn, _fn.data(), sizeof(TriFace) * _fn.size());
  }


  // Load the .mtl files
  if (loadMtl) {
    // get the path from filename
    char *mtlPathName = nullptr;
    const char *pathEnd = strrchr(filename, '\\');
    if (!pathEnd) pathEnd = strrchr(filename, '/');
    if (pathEnd) {
      int n = int(pathEnd - filename) + 1;
      mtlPathName = new char[n + 1];
      strncpy(mtlPathName, filename, n);
      mtlPathName[n] = '\0';
    }
    for (unsigned int mi = 0; mi < mtlFiles.size(); mi++) {
      std::string mtlFilename =
          (mtlPathName) ? std::string(mtlPathName) + mtlFiles[mi].filename
                        : mtlFiles[mi].filename;
      FILE *fp = fopen(mtlFilename.data(), "r");
      if (!fp) {
        if (outStream)
          *outStream << "ERROR: Cannot open file " << mtlFilename << std::endl;
        continue;
      }
      int mtlID = -1;
      while (buffer.ReadLine(fp)) {
        if (buffer.IsCommand("newmtl")) {
          mtlID = mtlList.GetMtlIndex(buffer.Data(7));
          if (mtlID >= 0) buffer.Copy(m[mtlID].name, 7);
        } else if (mtlID >= 0) {
          if (buffer.IsCommand("Ka")) buffer.ReadFloat3(m[mtlID].Ka);
          else if (buffer.IsCommand("Kd")) buffer.ReadFloat3(m[mtlID].Kd);
          else if (buffer.IsCommand("Ks")) buffer.ReadFloat3(m[mtlID].Ks);
          else if (buffer.IsCommand("Tf")) buffer.ReadFloat3(m[mtlID].Tf);
          else if (buffer.IsCommand("Ns")) buffer.ReadFloat(&m[mtlID].Ns);
          else if (buffer.IsCommand("Ni")) buffer.ReadFloat(&m[mtlID].Ni);
          else if (buffer.IsCommand("illum"))
            buffer.ReadInt(&m[mtlID].illum, 5);
          else if (buffer.IsCommand("map_Ka")) buffer.Copy(m[mtlID].map_Ka, 7);
          else if (buffer.IsCommand("map_Kd")) buffer.Copy(m[mtlID].map_Kd, 7);
          else if (buffer.IsCommand("map_Ks")) buffer.Copy(m[mtlID].map_Ks, 7);
          else if (buffer.IsCommand("map_Ns")) buffer.Copy(m[mtlID].map_Ns, 7);
          else if (buffer.IsCommand("map_d")) buffer.Copy(m[mtlID].map_d, 6);
          else if (buffer.IsCommand("map_bump"))
            buffer.Copy(m[mtlID].map_bump, 9);
          else if (buffer.IsCommand("bump")) buffer.Copy(m[mtlID].map_bump, 5);
          else if (buffer.IsCommand("map_disp"))
            buffer.Copy(m[mtlID].map_disp, 9);
          else if (buffer.IsCommand("disp")) buffer.Copy(m[mtlID].map_disp, 5);
        }
      }
      fclose(fp);
    }
    if (mtlPathName) delete[] mtlPathName;
  }

#endif

  return true;

}

//-------------------------------------------------------------------------------

inline bool TriMesh::SaveToFileObj(const char *filename,
                                   std::ostream *outStream)
{
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    if (outStream)
      *outStream << "ERROR: Cannot create file " << filename << std::endl;
    return false;
  }

  for (unsigned int i = 0; i < nv; i++) {
    fprintf(fp, "v %f %f %f\n", v[i].x, v[i].y, v[i].z);
  }
  for (unsigned int i = 0; i < nvt; i++) {
    fprintf(fp, "vt %f %f %f\n", vt[i].x, vt[i].y, vt[i].z);
  }
  for (unsigned int i = 0; i < nvn; i++) {
    fprintf(fp, "vn %f %f %f\n", vn[i].x, vn[i].y, vn[i].z);
  }
  int faceFormat = ((nvn > 0) << 1) | (nvt > 0);
  switch (faceFormat) {
    case 0:
      for (unsigned int i = 0; i < nf; i++) {
        fprintf(fp,
                "f %d %d %d\n",
                f[i].v[0] + 1,
                f[i].v[1] + 1,
                f[i].v[2] + 1);
      }
      break;
    case 1:
      for (unsigned int i = 0; i < nf; i++) {
        fprintf(fp,
                "f %d/%d %d/%d %d/%d\n",
                f[i].v[0] + 1,
                ft[i].v[0] + 1,
                f[i].v[1] + 1,
                ft[i].v[1] + 1,
                f[i].v[2] + 1,
                ft[i].v[2] + 1);
      }
      break;
    case 2:
      for (unsigned int i = 0; i < nf; i++) {
        fprintf(fp,
                "f %d//%d %d//%d %d//%d\n",
                f[i].v[0] + 1,
                fn[i].v[0] + 1,
                f[i].v[1] + 1,
                fn[i].v[1] + 1,
                f[i].v[2] + 1,
                fn[i].v[2] + 1);
      }
      break;
    case 3:
      for (unsigned int i = 0; i < nf; i++) {
        fprintf(fp,
                "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                f[i].v[0] + 1,
                ft[i].v[0] + 1,
                fn[i].v[0] + 1,
                f[i].v[1] + 1,
                ft[i].v[1] + 1,
                fn[i].v[1] + 1,
                f[i].v[2] + 1,
                ft[i].v[2] + 1,
                fn[i].v[2] + 1);
      }
      break;
  }

  fclose(fp);

  return true;
}

//-------------------------------------------------------------------------------
} // namespace cy
//-------------------------------------------------------------------------------

typedef cy::TriMesh cyTriMesh;    //!< Triangular Mesh Class

//-------------------------------------------------------------------------------

#endif

