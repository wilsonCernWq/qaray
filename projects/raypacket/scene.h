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


#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>


#include "core/sampler.h"
#include "core/camera.h"
#include "core/transform.h"
#include "core/box.h"
#include "core/ray.h"

#include "math/math.h"

#ifdef USE_TBB
# include <tbb/task_arena.h>
# include <tbb/task_scheduler_init.h>
# include <tbb/parallel_for.h>
# include <tbb/enumerable_thread_specific.h>
#endif

#ifdef USE_OMP
# include <omp.h>
#endif

//-----------------------------------------------------------------------------


//------------------------------------------------------------------------------

/* inline float Halton (int index, int base) */
/* { */
/*   float r = 0; */
/*   float f = 1.0f / (float) base; */
/*   for (int i = index; i > 0; i /= base) */
/*   { */
/*     r += f * (i % base); */
/*     f /= (float) base; */
/*   } */
/*   return r; */
/* } */
//
//struct HaltonRandom {
//  int idx = 0;
//  HaltonRandom(int);
//  void Seed(int seed);
//  void Get(float &r1, float &r2);
//  void Increment();
//};
//
//typedef tbb::enumerable_thread_specific<HaltonRandom> TBBHalton;
//extern TBBHalton TBBHaltonRNG;
//
//struct UniformRandom {
//  virtual float Get() = 0;
//};

typedef tbb::enumerable_thread_specific<Sampler*> TBBSampler;
extern TBBSampler rng;

//-----------------------------------------------------------------------------

class Node;

class HitInfo;

class DiffHitInfo;

bool TraceNodeShadow(Node &node, Ray &ray, HitInfo &hInfo);

bool TraceNodeNormal(Node &node, DiffRay &ray, DiffHitInfo &hInfo);

//-----------------------------------------------------------------------------

class SuperSampler {
 public:
  virtual const Color3f &GetColor() const = 0;

  virtual int GetSampleID() const = 0;

  virtual bool Loop() const = 0;

  virtual Point3 NewPixelSample() = 0;

  virtual Point3 NewDofSample(const float) = 0;

  virtual void Accumulate(const Color3f &localColor) = 0;

  virtual void Increment() = 0;
};

class SuperSamplerHalton : public SuperSampler {
 private:
  const Color3f th;
  const int sppMin, sppMax;
  Color3f color_std = Color3f(0.0f, 0.0f, 0.0f);
  Color3f color = Color3f(0.0f, 0.0f, 0.0f);
  int s = 0;
 public:
  SuperSamplerHalton(const Color3f th, const int sppMin, const int sppMax);

  const Color3f &GetColor() const;

  int GetSampleID() const;

  bool Loop() const;

  Point3 NewPixelSample();

  Point3 NewDofSample(const float);

  void Accumulate(const Color3f &localColor);

  void Increment();
};

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

class Node;

struct HitInfoCore {
  float z;        // the distance from the ray center to the hit point
  Point3 p;        // position of the hit point
  Point3 N;        // surface normal at the hit point
  HitInfoCore() { Init(); }

  void Init()
  {
    z = BIGFLOAT;
  }
};

struct HitInfo {
  float z;            // the distance from the ray center to the hit point
  Point3 p;           // position of the hit point
  Point3 N;           // surface normal at the hit point
  Point3 uvw;         // texture coordinate at the hit point
  Point3 duvw[2];     // derivatives of the texture coordinate
  int mtlID;          // sub-material index
  const Node *
      node;   // the object node that was hit, false if the ray hits the back side
  //------------------//
  bool hasFrontHit;   // true if the ray hits the front side,
  bool hasTexture;
  //------------------

  HitInfo() { Init(); }

  void Init()
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

struct DiffHitInfo {
  HitInfo c;
  HitInfoCore x, y;

  DiffHitInfo() { Init(); }

  void Init()
  {
    c.Init();
    x.Init();
    y.Init();
  }
};
//-----------------------------------------------------------------------------

class ItemBase {
 private:
  char *name;          // The name of the item

 public:
  ItemBase() : name(NULL) {}

  virtual ~ItemBase() { if (name) delete[] name; }

  const char *GetName() const { return name ? name : ""; }

  void SetName(const char *newName)
  {
    if (name) delete[] name;
    if (newName) {
      int n = strlen(newName);
      name = new char[n + 1];
      for (int i = 0; i < n; i++) name[i] = newName[i];
      name[n] = '\0';
    } else { name = NULL; }
  }
};

template<class T>
class ItemList : public std::vector<T *> {
 public:
  virtual ~ItemList() { DeleteAll(); }

  void DeleteAll()
  {
    int n = (int) this->size();
    for (int i = 0; i < n; i++) if (this->at(i)) delete this->at(i);
  }
};

template<class T>
class ItemFileList {
 public:
  void Clear() { list.DeleteAll(); }

  void Append(T *item, const char *name)
  {
    list.push_back(new FileInfo(item, name));
  }

  T *Find(const char *name) const
  {
    int n = list.size();
    for (int i = 0; i < n; i++)
      if (list[i] && strcmp(name, list[i]->GetName()) == 0)
        return list[i]->GetObj();
    return NULL;
  }

 private:
  class FileInfo : public ItemBase {
   private:
    T *item;
   public:
    FileInfo() : item(NULL) {}

    FileInfo(T *_item, const char *name) : item(_item) { SetName(name); }

    ~FileInfo() { Delete(); }

    void Delete()
    {
      if (item) delete item;
      item = NULL;
    }

    void SetObj(T *_item)
    {
      Delete();
      item = _item;
    }

    T *GetObj() { return item; }
  };

  ItemList<FileInfo> list;
};

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

class Material;

// Base class for all object types
class Object {
 public:
  virtual ~Object() {}

  virtual bool IntersectRay(const Ray &ray,
                            HitInfo &hInfo,
                            int hitSide = HIT_FRONT,
                            DiffRay *diffray = NULL,
                            DiffHitInfo *diffhit = NULL) const =0;

  virtual Box GetBoundBox() const =0;

  virtual void ViewportDisplay(const Material *mtl) const {}  // used for OpenGL display
};

typedef ItemFileList<Object> ObjFileList;

//-----------------------------------------------------------------------------

class Light : public ItemBase {
 public:
  virtual Color3f Illuminate(const Point3 &p, const Point3 &N) const =0;

  virtual Point3 Direction(const Point3 &p) const =0;

  virtual bool IsAmbient() const { return false; }

  virtual void SetViewportLight(int lightID) const {}  // used for OpenGL display
};

class LightList : public ItemList<Light> {
};

//-----------------------------------------------------------------------------

class Material : public ItemBase {
 public:
  static int maxBounce;
 public:
  // The main method that handles the shading by calling all the lights in the list.
  // ray: incoming ray,
  // hInfo: hit information for the point that is being shaded, lights: the light list,
  // bounceCount: permitted number of additional bounces for reflection and refraction.
  virtual Color3f Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                      const LightList &lights, int bounceCount) const =0;

  virtual void SetViewportMaterial(int subMtlID = 0) const {}  // used for OpenGL display
};

class MaterialList : public ItemList<Material> {
 public:
  Material *Find(const char *name)
  {
    int n = size();
    for (int i = 0; i < n; i++)
      if (at(i) && strcmp(name, at(i)->GetName()) == 0)return at(i);
    return NULL;
  }
};

//-------------------------------------------------------------------------------

class Texture : public ItemBase {
 public:
  // Evaluates the color at the given uvw location.
  virtual Color3f Sample(const Point3 &uvw) const =0;

  // Evaluates the color around the given uvw location using the derivatives duvw
  // by calling the Sample function multiple times.
  virtual Color3f Sample(const Point3 &uvw,
                       const Point3 duvw[2],
                       bool elliptic = true) const
  {
    Color3f c = Sample(uvw);
    if (glm::length2(duvw[0]) + glm::length2(duvw[1]) == 0) return c;
    for (int i = 1; i < TEXTURE_SAMPLE_COUNT; i++) {
      float x = Halton(i, 2);
      float y = Halton(i, 3);
      if (elliptic) {
        float r = sqrtf(x) * 0.5f;
        x = r * sinf(y * (float) M_PI * 2);
        y = r * cosf(y * (float) M_PI * 2);
      } else {
        if (x > 0.5f) x -= 1;
        if (y > 0.5f) y -= 1;
      }
      c += Sample(uvw + x * duvw[0] + y * duvw[1]);
    }
    return c / float(TEXTURE_SAMPLE_COUNT);
  }

  virtual bool SetViewportTexture() const { return false; }// used for OpenGL display
 protected:
  // Clamps the uvw values for tiling textures, such that all values fall between 0 and 1.
  static Point3 TileClamp(const Point3 &uvw)
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
class TextureMap : public Transformation {
 public:
  TextureMap() : texture(NULL) {}

  TextureMap(Texture *tex) : texture(tex) {}

  void SetTexture(Texture *tex) { texture = tex; }

  virtual Color3f Sample(const Point3 &uvw) const
  {
    return texture ? texture->Sample(TransformTo(uvw)) : Color3f(0, 0, 0);
  }

  virtual Color3f Sample(const Point3 &uvw,
                       const Point3 duvw[2],
                       bool elliptic = true) const
  {
    if (texture == NULL) return Color3f(0, 0, 0);
    Point3 u = TransformTo(uvw);
    Point3 d[2];
    d[0] = TransformTo(duvw[0] + uvw) - u;
    d[1] = TransformTo(duvw[1] + uvw) - u;
    return texture->Sample(u, d, elliptic);
  }

  bool SetViewportTexture() const
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
class TexturedColor {
 private:
  Color3f color;
  TextureMap *map;
 public:
  TexturedColor() : color(0, 0, 0), map(NULL) {}

  TexturedColor(float r, float g, float b) : color(r, g, b), map(NULL) {}

  virtual ~TexturedColor() { if (map) delete map; }

  void SetColor(const Color3f &c) { color = c; }

  void SetTexture(TextureMap *m)
  {
    if (map) delete map;
    map = m;
  }

  Color3f GetColor() const { return color; }

  const TextureMap *GetTexture() const { return map; }

  Color3f Sample(const Point3 &uvw) const
  {
    return (map) ? color * map->Sample(uvw) : color;
  }

  Color3f Sample(const Point3 &uvw,
               const Point3 duvw[2],
               bool elliptic = true) const
  {
    return (map) ? color * map->Sample(uvw, duvw, elliptic) : color;
  }

  // Returns the color value at the given direction for environment mapping.
  Color3f SampleEnvironment(const Point3 &dir) const
  {
    float z = asinf(-dir.z) / float(M_PI) + 0.5f;
    float x = dir.x / (fabs(dir.x) + fabs(dir.y));
    float y = dir.y / (fabs(dir.x) + fabs(dir.y));
    return Sample(Point3(0.5f, 0.5f, 0.0f) + z
        * (x * Point3(0.5f, 0.5f, 0) + y * Point3(-0.5f, 0.5f, 0)));
  }
};

//-----------------------------------------------------------------------------

class Node : public ItemBase, public Transformation {
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
  Node() : child(NULL), numChild(0), obj(NULL), mtl(NULL) {}

  virtual ~Node() { DeleteAllChildNodes(); }

  // Initialize the node deleting all child nodes
  void Init()
  {
    DeleteAllChildNodes();
    obj = NULL;
    mtl = NULL;
    SetName(NULL);
    InitTransform();
  }

  // Hierarchy management
  int GetNumChild() const { return numChild; }

  void SetNumChild(int n, int keepOld = false)
  {
    if (n < 0) n = 0;  // just to be sure
    Node **nc = NULL;  // new child pointer
    if (n > 0) nc = new Node *[n];
    for (int i = 0; i < n; i++) nc[i] = NULL;
    if (keepOld) {
      int sn = MIN(n, numChild);
      for (int i = 0; i < sn; i++) nc[i] = child[i];
    }
    if (child) delete[] child;
    child = nc;
    numChild = n;
  }

  const Node *GetChild(int i) const { return child[i]; }

  Node *GetChild(int i) { return child[i]; }

  void SetChild(int i, Node *node) { child[i] = node; }

  void AppendChild(Node *node)
  {
    SetNumChild(numChild + 1, true);
    SetChild(numChild - 1, node);
  }

  void RemoveChild(int i)
  {
    for (int j = i; j < numChild - 1; j++) child[j] = child[j + 1];
    SetNumChild(numChild - 1);
  }

  void DeleteAllChildNodes()
  {
    for (int i = 0; i < numChild; i++) {
      child[i]->DeleteAllChildNodes();
      delete child[i];
    }
    SetNumChild(0);
  }

  // Bounding Box
  const Box &ComputeChildBoundBox()
  {
    childBoundBox.Init();
    for (int i = 0; i < numChild; i++) {
      Box childBox = child[i]->ComputeChildBoundBox();
      Object *cobj = child[i]->GetNodeObj();
      if (cobj) childBox += cobj->GetBoundBox();
      if (!childBox.IsEmpty()) {
        // transform the box from child coordinates
        for (int j = 0; j < 8; j++)
          childBoundBox += child[i]->TransformFrom(childBox.Corner(j));
      }
    }
    return childBoundBox;
  }

  const Box &GetChildBoundBox() const { return childBoundBox; }

  // Object management
  const Object *GetNodeObj() const { return obj; }

  Object *GetNodeObj() { return obj; }

  void SetNodeObj(Object *object) { obj = object; }

  // Material management
  const Material *GetMaterial() const { return mtl; }

  void SetMaterial(Material *material) { mtl = material; }

  // Transformations
  Ray ToNodeCoords(const Ray &ray) const
  {
    Ray r;
    r.p = TransformTo(ray.p);
    r.dir = TransformTo(ray.p + ray.dir) - r.p;
    return r;
  }

  DiffRay ToNodeCoords(const DiffRay &ray) const
  {
    DiffRay r;
    r.c = ToNodeCoords(ray.c);
    r.x = ToNodeCoords(ray.x);
    r.y = ToNodeCoords(ray.y);
    return r;
  }

  void FromNodeCoords(HitInfo &hInfo) const
  {
    hInfo.p = TransformFrom(hInfo.p);
    hInfo.N = glm::normalize(VectorTransformFrom(hInfo.N));
  }

  void FromNodeCoords(DiffHitInfo &hInfo) const
  {
    FromNodeCoords(hInfo.c);
    hInfo.x.p = TransformFrom(hInfo.x.p);
    hInfo.x.N = glm::normalize(VectorTransformFrom(hInfo.x.N));
    hInfo.y.p = TransformFrom(hInfo.y.p);
    hInfo.y.N = glm::normalize(VectorTransformFrom(hInfo.y.N));
  }
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


#endif
