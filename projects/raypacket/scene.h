//-----------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 6, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-----------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

//-----------------------------------------------------------------------------

#define TEXTURE_SAMPLE_COUNT 32
#ifndef _USE_MATH_DEFINES
# define _USE_MATH_DEFINES
#endif

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <algorithm>
#include "math/math.h"

//-----------------------------------------------------------------------------

#define debug(x) (std::cout << #x << " " << x << std::endl)
#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)
#define CLAMP(X, L, U) MIN(U, MAX(L, X))
#define ABS(x)    (x > 0 ? x :-x)
#define POW(x, y) (std::pow(x, y))
#define SQRT(x) (std::sqrt(x))
#define CEIL(x)  (std::ceil(x))
#define FLOOR(x) (std::floor(x))
#define SIN(x) (std::sin(x))
#define COS(x) (std::cos(x))
#define BIGFLOAT 1.0e30f

//------------------------------------------------------------------------------

inline float Halton (int index, int base)
{
  float r = 0;
  float f = 1.0f / (float) base;
  for (int i = index; i > 0; i /= base)
  {
    r += f * (i % base);
    f /= (float) base;
  }
  return r;
}

struct UniformRandom
{
  virtual double Get () = 0;
};

extern UniformRandom *rng;

inline Point3 GetCirclePoint (const float r1, const float r2, const float r3, float size)
{
  Point3 p;
  do
  {
    p.x = (2.f * r1 - 1.f) * size;
    p.y = (2.f * r2 - 1.f) * size;
    p.z = (2.f * r3 - 1.f) * size;
  } while (glm::length(p) > size);
  return p;
}

inline Point3 UniformSampleHemiSphere (const float r1, const float r2)
{
  // Generate random direction on unit hemisphere proportional to solid angle
  // PDF = 1 / 2PI 
  const float cosTheta = r1;
  const float sinTheta = SQRT(1 - r1 * r1);
  const float phi = 2 * M_PI * r2;
  const float x = sinTheta * COS(phi);
  const float y = sinTheta * SIN(phi);
  return Point3(x, y, cosTheta);
}

inline Point3 CosWeightedSampleHemiSphere (const float r1, const float r2)
{
  // Generate random direction on unit hemisphere proportional to cosine-weighted solid angle
  // PDF = cos(Theta) / PI
  const float cosTheta = SQRT(r1);
  const float sinTheta = SQRT(1 - r1);
  const float phi = 2 * M_PI * r2;
  const float x = sinTheta * COS(phi);
  const float y = sinTheta * SIN(phi);
  return Point3(x, y, cosTheta);
}

inline Point3 CosLobeWeightedSampleHemiSphere (const float r1, const float r2,
                                               const int N, const int theta_max = 90)
{
  // Generate random direction on unit hemisphere proportional to cosine lobe around normal
  if (theta_max == 90)
  {
    // PDF = (N+1) * (cos(theta) ^ N) / 2PI
    const float cosTheta = POW(r1, 1.f / (N + 1));
    const float sinTheta = SQRT(1 - cosTheta * cosTheta);
    const float phi = 2 * M_PI * r2;
    const float x = sinTheta * COS(phi);
    const float y = sinTheta * SIN(phi);
    return Point3(x, y, cosTheta);
  } else
  {
    // PDF = (N+1) * (cos(theta) ^ N) / 2PI * (theta_max / 90)
    const float cosTheta = POW(1.f - r1 * (1.f - POW(COS(theta_max), N + 1)), 1.f / (N + 1));
    const float sinTheta = SQRT(1 - cosTheta * cosTheta);
    const float phi = 2 * M_PI * r2;
    const float x = sinTheta * COS(phi);
    const float y = sinTheta * SIN(phi);
    return Point3(x, y, cosTheta);
  }
}

//-----------------------------------------------------------------------------

class Node;

class Ray;

class DiffRay;

class HitInfo;

class DiffHitInfo;

bool TraceNodeShadow (Node &node, Ray &ray, HitInfo &hInfo);

bool TraceNodeNormal (Node &node, DiffRay &ray, DiffHitInfo &hInfo);

//-----------------------------------------------------------------------------

class SuperSampler
{
public:
  virtual const Color &GetColor () const = 0;

  virtual const int GetSampleID () const = 0;

  virtual bool Loop () const = 0;

  virtual Point3 NewPixelSample () = 0;

  virtual Point3 NewDofSample (const float) = 0;

  virtual void Accumulate (const Color &localColor) = 0;

  virtual void Increment () = 0;
};

class SuperSamplerHalton : public SuperSampler
{
private:
  const Color th;
  const int sppMin, sppMax;
  Color color_std = Color(0.0f, 0.0f, 0.0f);
  Color color = Color(0.0f, 0.0f, 0.0f);
  int s = 0;
public:
  SuperSamplerHalton (const Color th, const int sppMin, const int sppMax);

  const Color &GetColor () const;

  const int GetSampleID () const;

  bool Loop () const;

  Point3 NewPixelSample ();

  Point3 NewDofSample (const float);

  void Accumulate (const Color &localColor);

  void Increment ();
};

//-----------------------------------------------------------------------------

class Box
{
public:
  Point3 pmin, pmax;

  // Constructors
  Box () { Init(); }

  Box (const Point3 &_pmin, const Point3 &_pmax) : pmin(_pmin), pmax(_pmax) {}

  Box (float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) :
      pmin(xmin, ymin, zmin), pmax(xmax, ymax, zmax) {}

  Box (const float *dim) :
      pmin(dim[0], dim[1], dim[2]), pmax(dim[3], dim[4], dim[5]) {}

  // Initializes the box, such that there exists no point inside the box (i.e. it is empty).
  void Init ()
  {
    pmin = Point3(BIGFLOAT, BIGFLOAT, BIGFLOAT);
    pmax = Point3(-BIGFLOAT, -BIGFLOAT, -BIGFLOAT);
  }

  // Returns true if the box is empty; otherwise, returns false.
  bool IsEmpty () const { return pmin.x > pmax.x || pmin.y > pmax.y || pmin.z > pmax.z; }

  // Returns one of the 8 corner point of the box in the following order:
  // 0:(x_min,y_min,z_min), 1:(x_max,y_min,z_min)
  // 2:(x_min,y_max,z_min), 3:(x_max,y_max,z_min)
  // 4:(x_min,y_min,z_max), 5:(x_max,y_min,z_max)
  // 6:(x_min,y_max,z_max), 7:(x_max,y_max,z_max)
  Point3 Corner (int i) const  // 8 corners of the box
  {
    Point3 p;
    p.x = (i & 1) ? pmax.x : pmin.x;
    p.y = (i & 2) ? pmax.y : pmin.y;
    p.z = (i & 4) ? pmax.z : pmin.z;
    return p;
  }

  // Enlarges the box such that it includes the given point p.
  void operator+= (const Point3 &p)
  {
    for (int i = 0; i < 3; i++)
    {
      if (pmin[i] > p[i]) pmin[i] = p[i];
      if (pmax[i] < p[i]) pmax[i] = p[i];
    }
  }

  // Enlarges the box such that it includes the given box b.
  void operator+= (const Box &b)
  {
    for (int i = 0; i < 3; i++)
    {
      if (pmin[i] > b.pmin[i]) pmin[i] = b.pmin[i];
      if (pmax[i] < b.pmax[i]) pmax[i] = b.pmax[i];
    }
  }

  // Returns true if the point is inside the box; otherwise, returns false.
  bool IsInside (const Point3 &p) const
  {
    for (int i = 0; i < 3; i++)
      if (pmin[i] > p[i] || pmax[i] < p[i]) return false;
    return true;
  }

  // Returns true if the ray intersects with the box for any parameter that
  // is smaller than t_max; otherwise, returns false.
  bool IntersectRay (const Ray &r, float t_max) const;
};

//-----------------------------------------------------------------------------

class HitInfo;

struct Ray
{
  Point3 p, dir;

  Ray () {}

  Ray (const Point3 &p, const Point3 &d) : p(p), dir(d) {}

  Ray (const Ray &r) :
      p(r.p), dir(r.dir) {}

  void Normalize () { dir = glm::normalize(dir); }
};

struct DiffRay
{
  static const float dx, dy, rdx, rdy;
  Ray c, x, y;
  bool hasDiffRay = true;

  DiffRay () = default;

  DiffRay (const Point3 &p, const Point3 &d) :
      c(p, d), x(p, d), y(p, d), hasDiffRay(false) {}

  DiffRay (const Point3 &pc, const Point3 &dc,
           const Point3 &px, const Point3 &dx,
           const Point3 &py, const Point3 &dy) :
      c(pc, dc), x(px, dx), y(py, dy), hasDiffRay(true) {}

  DiffRay (const DiffRay &r) : c(r.c), x(r.x), y(r.y), hasDiffRay(r.hasDiffRay) {}

  void Normalize ()
  {
    c.Normalize();
    x.Normalize();
    y.Normalize();
  }
};

//-----------------------------------------------------------------------------

class Node;

struct HitInfoCore
{
  float z;        // the distance from the ray center to the hit point
  Point3 p;        // position of the hit point
  Point3 N;        // surface normal at the hit point
  HitInfoCore () { Init(); }

  void Init ()
  {
    z = BIGFLOAT;
  }
};

struct HitInfo
{
  float z;        // the distance from the ray center to the hit point
  Point3 p;        // position of the hit point
  Point3 N;        // surface normal at the hit point
  Point3 uvw;         // texture coordinate at the hit point
  Point3 duvw[2];     // derivatives of the texture coordinate
  int mtlID;          // sub-material index
  const Node *node;   // the object node that was hit, false if the ray hits the back side
  //------------------//
  bool hasFrontHit;   // true if the ray hits the front side,
  bool hasTexture;

  HitInfo () { Init(); }

  void Init ()
  {
    z = BIGFLOAT;
    uvw = Point3(0.5f, 0.5f, 0.5f);
    duvw[0] = Point3(0.0f);
    duvw[1] = Point3(0.0f);
    node = NULL;
    mtlID = 0;
    hasFrontHit = true;
    hasTexture = false;
  }
};

struct DiffHitInfo
{
  HitInfo c;
  HitInfoCore x, y;

  DiffHitInfo () { Init(); }

  void Init ()
  {
    c.Init();
    x.Init();
    y.Init();
  }
};
//-----------------------------------------------------------------------------

class ItemBase
{
private:
  char *name;          // The name of the item

public:
  ItemBase () : name(NULL) {}

  virtual ~ItemBase () { if (name) delete[] name; }

  const char *GetName () const { return name ? name : ""; }

  void SetName (const char *newName)
  {
    if (name) delete[] name;
    if (newName)
    {
      int n = strlen(newName);
      name = new char[n + 1];
      for (int i = 0; i < n; i++) name[i] = newName[i];
      name[n] = '\0';
    } else { name = NULL; }
  }
};

template<class T>
class ItemList : public std::vector<T *>
{
public:
  virtual ~ItemList () { DeleteAll(); }

  void DeleteAll ()
  {
    int n = (int) this->size();
    for (int i = 0; i < n; i++) if (this->at(i)) delete this->at(i);
  }
};

template<class T>
class ItemFileList
{
public:
  void Clear () { list.DeleteAll(); }

  void Append (T *item, const char *name) { list.push_back(new FileInfo(item, name)); }

  T *Find (const char *name) const
  {
    int n = list.size();
    for (int i = 0; i < n; i++)
      if (list[i] && strcmp(name, list[i]->GetName()) == 0) return list[i]->GetObj();
    return NULL;
  }

private:
  class FileInfo : public ItemBase
  {
  private:
    T *item;
  public:
    FileInfo () : item(NULL) {}

    FileInfo (T *_item, const char *name) : item(_item) { SetName(name); }

    ~FileInfo () { Delete(); }

    void Delete ()
    {
      if (item) delete item;
      item = NULL;
    }

    void SetObj (T *_item)
    {
      Delete();
      item = _item;
    }

    T *GetObj () { return item; }
  };

  ItemList<FileInfo> list;
};

//-----------------------------------------------------------------------------

class Transformation
{
private:
  Matrix3 tm;    // Transformation matrix to the local space
  Point3 pos;    // Translation part of the transformation matrix
  mutable Matrix3 itm;  // Inverse of the transformation matrix (cached)
public:
  Transformation () : pos(0, 0, 0)
  {
    tm = Matrix3(1.f);
    itm = Matrix3(1.f);
  }

  const Matrix3 &GetTransform () const { return tm; }

  const Point3 &GetPosition () const { return pos; }

  const Matrix3 &GetInverseTransform () const { return itm; }

  // Transform to the local coordinate system
  Point3 TransformTo (const Point3 &p) const { return itm * (p - pos); }

  // Transform from the local coordinate system
  Point3 TransformFrom (const Point3 &p) const { return tm * p + pos; }

  // Transforms a vector to the local coordinate system
  // (same as multiplication with the inverse transpose of the transformation)
  Point3 VectorTransformTo (const Point3 &dir) const { return TransposeMult(tm, dir); }

  // Transforms a vector from the local coordinate system
  // (same as multiplication with the inverse transpose of the transformation)
  Point3 VectorTransformFrom (const Point3 &dir) const { return TransposeMult(itm, dir); }

  void Translate (Point3 p) { pos += p; }

  void Rotate (Point3 axis, float degree)
  {
    Matrix3 m(glm::rotate(Matrix4(1.0f), degree * (float) M_PI / 180.0f, axis));
    Transform(m);
  }

  void Scale (float sx, float sy, float sz)
  {
    Matrix3 m(glm::scale(Matrix4(1.0f), Point3(sx, sy, sz)));
    Transform(m);
  }

  void Transform (const Matrix3 &m)
  {
    tm = m * tm;
    pos = m * pos;
    itm = glm::inverse(tm);
  }

  void InitTransform ()
  {
    pos = Point3(0.f);
    tm = Matrix3(1.f);
    itm = Matrix3(1.f);
  }


private:
  // Multiplies the given vector with the transpose of the given matrix
  static Point3 TransposeMult (const Matrix3 &m, const Point3 &dir)
  {
    Point3 d;
    d.x = glm::dot(glm::column(m, 0), dir);
    d.y = glm::dot(glm::column(m, 1), dir);
    d.z = glm::dot(glm::column(m, 2), dir);
    return d;
  }
};

//-----------------------------------------------------------------------------

class Material;

// Base class for all object types
class Object
{
public:
  virtual ~Object () {}

  virtual bool IntersectRay (const Ray &ray, HitInfo &hInfo, int hitSide = HIT_FRONT,
                             DiffRay *diffray = NULL, DiffHitInfo *diffhit = NULL) const =0;

  virtual Box GetBoundBox () const =0;

  virtual void ViewportDisplay (const Material *mtl) const {}  // used for OpenGL display
};

typedef ItemFileList<Object> ObjFileList;

//-----------------------------------------------------------------------------

class Light : public ItemBase
{
public:
  virtual Color Illuminate (const Point3 &p, const Point3 &N) const =0;

  virtual Point3 Direction (const Point3 &p) const =0;

  virtual bool IsAmbient () const { return false; }

  virtual void SetViewportLight (int lightID) const {}  // used for OpenGL display
};

class LightList : public ItemList<Light>
{
};

//-----------------------------------------------------------------------------

class Material : public ItemBase
{
public:
  static int maxBounce;
  static float gamma;
  static bool sRGB;
public:
  // The main method that handles the shading by calling all the lights in the list.
  // ray: incoming ray,
  // hInfo: hit information for the point that is being shaded, lights: the light list,
  // bounceCount: permitted number of additional bounces for reflection and refraction.
  virtual Color Shade (const DiffRay &ray, const DiffHitInfo &hInfo,
                       const LightList &lights, int bounceCount) const =0;

  virtual void SetViewportMaterial (int subMtlID = 0) const {}  // used for OpenGL display
};

class MaterialList : public ItemList<Material>
{
public:
  Material *Find (const char *name)
  {
    int n = size();
    for (int i = 0; i < n; i++) if (at(i) && strcmp(name, at(i)->GetName()) == 0) return at(i);
    return NULL;
  }
};

//-------------------------------------------------------------------------------

class Texture : public ItemBase
{
public:
  // Evaluates the color at the given uvw location.
  virtual Color Sample (const Point3 &uvw) const =0;

  // Evaluates the color around the given uvw location using the derivatives duvw
  // by calling the Sample function multiple times.
  virtual Color Sample (const Point3 &uvw, const Point3 duvw[2], bool elliptic = true) const
  {
    Color c = Sample(uvw);
    if (glm::length2(duvw[0]) + glm::length2(duvw[1]) == 0) return c;
    for (int i = 1; i < TEXTURE_SAMPLE_COUNT; i++)
    {
      float x = Halton(i, 2);
      float y = Halton(i, 3);
      if (elliptic)
      {
        float r = sqrtf(x) * 0.5f;
        x = r * sinf(y * (float) M_PI * 2);
        y = r * cosf(y * (float) M_PI * 2);
      } else
      {
        if (x > 0.5f) x -= 1;
        if (y > 0.5f) y -= 1;
      }
      c += Sample(uvw + x * duvw[0] + y * duvw[1]);
    }
    return c / float(TEXTURE_SAMPLE_COUNT);
  }

  virtual bool SetViewportTexture () const { return false; }// used for OpenGL display
protected:
  // Clamps the uvw values for tiling textures, such that all values fall between 0 and 1.
  static Point3 TileClamp (const Point3 &uvw)
  {
    Point3 u;
    u.x = uvw.x - (int) uvw.x;
    u.y = uvw.y - (int) uvw.y;
    u.z = uvw.z - (int) uvw.z;
    if (u.x < 0) u.x += 1;
    if (u.y < 0) u.y += 1;
    if (u.z < 0) u.z += 1;
    return u;
  }
};

typedef ItemFileList<Texture> TextureList;

//-------------------------------------------------------------------------------

// This class handles textures with texture transformations.
// The uvw values passed to the Sample methods are transformed
// using the texture transformation.
class TextureMap : public Transformation
{
public:
  TextureMap () : texture(NULL) {}

  TextureMap (Texture *tex) : texture(tex) {}

  void SetTexture (Texture *tex) { texture = tex; }

  virtual Color Sample (const Point3 &uvw) const
  {
    return texture ? texture->Sample(TransformTo(uvw)) : Color(0, 0, 0);
  }

  virtual Color Sample (const Point3 &uvw, const Point3 duvw[2], bool elliptic = true) const
  {
    if (texture == NULL) return Color(0, 0, 0);
    Point3 u = TransformTo(uvw);
    Point3 d[2];
    d[0] = TransformTo(duvw[0] + uvw) - u;
    d[1] = TransformTo(duvw[1] + uvw) - u;
    return texture->Sample(u, d, elliptic);
  }

  bool SetViewportTexture () const
  {
    if (texture) return texture->SetViewportTexture();
    return false;
  }// used for OpenGL display
private:
  Texture *texture;
};

//-------------------------------------------------------------------------------

// This class keeps a TextureMap and a color. This is useful for keeping material
// color parameters that can also be textures. If no texture is specified, it
// automatically uses the color value. Otherwise, the texture value is multiplied
// by the color value.
class TexturedColor
{
private:
  Color color;
  TextureMap *map;
public:
  TexturedColor () : color(0, 0, 0), map(NULL) {}

  TexturedColor (float r, float g, float b) : color(r, g, b), map(NULL) {}

  virtual ~TexturedColor () { if (map) delete map; }

  void SetColor (const Color &c) { color = c; }

  void SetTexture (TextureMap *m)
  {
    if (map) delete map;
    map = m;
  }

  Color GetColor () const { return color; }

  const TextureMap *GetTexture () const { return map; }

  Color Sample (const Point3 &uvw) const { return (map) ? color * map->Sample(uvw) : color; }

  Color Sample (const Point3 &uvw, const Point3 duvw[2], bool elliptic = true) const
  {
    return (map) ? color * map->Sample(uvw, duvw, elliptic) : color;
  }

  // Returns the color value at the given direction for environment mapping.
  Color SampleEnvironment (const Point3 &dir) const
  {
    float z = asinf(-dir.z) / float(M_PI) + 0.5f;
    float x = dir.x / (fabs(dir.x) + fabs(dir.y));
    float y = dir.y / (fabs(dir.x) + fabs(dir.y));
    return Sample(Point3(0.5f, 0.5f, 0.0f) + z * (x * Point3(0.5f, 0.5f, 0) + y * Point3(-0.5f, 0.5f, 0)));
  }
};

//-----------------------------------------------------------------------------

class Node : public ItemBase, public Transformation
{
private:
  Node **child;    // Child nodes
  int numChild;    // The number of child nodes
  Object *obj;    // Object reference
  // (merely points to the object, but does not own the object,
  //  so it doesn't get deleted automatically)
  Material *mtl;  // Material used for shading the object
  Box childBoundBox;  // Bounding box of the child nodes,
  // which does not include the object of this node,
  // but includes the objects of the child nodes
public:
  Node () : child(NULL), numChild(0), obj(NULL), mtl(NULL) {}

  virtual ~Node () { DeleteAllChildNodes(); }

  // Initialize the node deleting all child nodes
  void Init ()
  {
    DeleteAllChildNodes();
    obj = NULL;
    mtl = NULL;
    SetName(NULL);
    InitTransform();
  }

  // Hierarchy management
  int GetNumChild () const { return numChild; }

  void SetNumChild (int n, int keepOld = false)
  {
    if (n < 0) n = 0;  // just to be sure
    Node **nc = NULL;  // new child pointer
    if (n > 0) nc = new Node *[n];
    for (int i = 0; i < n; i++) nc[i] = NULL;
    if (keepOld)
    {
      int sn = MIN(n, numChild);
      for (int i = 0; i < sn; i++) nc[i] = child[i];
    }
    if (child) delete[] child;
    child = nc;
    numChild = n;
  }

  const Node *GetChild (int i) const { return child[i]; }

  Node *GetChild (int i) { return child[i]; }

  void SetChild (int i, Node *node) { child[i] = node; }

  void AppendChild (Node *node)
  {
    SetNumChild(numChild + 1, true);
    SetChild(numChild - 1, node);
  }

  void RemoveChild (int i)
  {
    for (int j = i; j < numChild - 1; j++) child[j] = child[j + 1];
    SetNumChild(numChild - 1);
  }

  void DeleteAllChildNodes ()
  {
    for (int i = 0; i < numChild; i++)
    {
      child[i]->DeleteAllChildNodes();
      delete child[i];
    }
    SetNumChild(0);
  }

  // Bounding Box
  const Box &ComputeChildBoundBox ()
  {
    childBoundBox.Init();
    for (int i = 0; i < numChild; i++)
    {
      Box childBox = child[i]->ComputeChildBoundBox();
      Object *cobj = child[i]->GetNodeObj();
      if (cobj) childBox += cobj->GetBoundBox();
      if (!childBox.IsEmpty())
      {
        // transform the box from child coordinates
        for (int j = 0; j < 8; j++)
          childBoundBox += child[i]->TransformFrom(childBox.Corner(j));
      }
    }
    return childBoundBox;
  }

  const Box &GetChildBoundBox () const { return childBoundBox; }

  // Object management
  const Object *GetNodeObj () const { return obj; }

  Object *GetNodeObj () { return obj; }

  void SetNodeObj (Object *object) { obj = object; }

  // Material management
  const Material *GetMaterial () const { return mtl; }

  void SetMaterial (Material *material) { mtl = material; }

  // Transformations
  Ray ToNodeCoords (const Ray &ray) const
  {
    Ray r;
    r.p = TransformTo(ray.p);
    r.dir = TransformTo(ray.p + ray.dir) - r.p;
    return r;
  }

  DiffRay ToNodeCoords (const DiffRay &ray) const
  {
    DiffRay r;
    r.c = ToNodeCoords(ray.c);
    r.x = ToNodeCoords(ray.x);
    r.y = ToNodeCoords(ray.y);
    return r;
  }

  void FromNodeCoords (HitInfo &hInfo) const
  {
    hInfo.p = TransformFrom(hInfo.p);
    hInfo.N = glm::normalize(VectorTransformFrom(hInfo.N));
  }

  void FromNodeCoords (DiffHitInfo &hInfo) const
  {
    FromNodeCoords(hInfo.c);
    hInfo.x.p = TransformFrom(hInfo.x.p);
    hInfo.x.N = glm::normalize(VectorTransformFrom(hInfo.x.N));
    hInfo.y.p = TransformFrom(hInfo.y.p);
    hInfo.y.N = glm::normalize(VectorTransformFrom(hInfo.y.N));
  }
};

//-----------------------------------------------------------------------------

class Camera
{
public:
  Point3 pos, dir, up;
  float fov, focaldist, dof;
  int imgWidth, imgHeight;

  void Init ()
  {
    pos = Point3(0, 0, 0);
    dir = Point3(0, 0, -1);
    up = Point3(0, 1, 0);
    fov = 40;
    focaldist = 1;
    dof = 0;
    imgWidth = 200;
    imgHeight = 150;
  }
};

//-----------------------------------------------------------------------------


#endif
