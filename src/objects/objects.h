//------------------------------------------------------------------------------
///
/// \file       objects.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    7.0
/// \date       October 6, 2015
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#ifndef _OBJECTS_H_INCLUDED_
#define _OBJECTS_H_INCLUDED_

#include "math/math.h"
#include "core/core.h"
#include "samplers/sampler_selection.h"

#include "cyTriMesh.h"
#include "cyBVH.h"

//------------------------------------------------------------------------------

class Sphere : public Object {
 public:
  virtual bool IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide,
                            DiffRay *diffray, DiffHitInfo *diffhit) const;

  virtual Box GetBoundBox() const { return Box(-1, -1, -1, 1, 1, 1); }

  virtual void ViewportDisplay(const Material *mtl) const;
};

extern Sphere theSphere;

//------------------------------------------------------------------------------

class Plane : public Object {
 public:
  virtual bool IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide,
                            DiffRay *diffray, DiffHitInfo *diffhit) const;

  virtual Box GetBoundBox() const { return Box(-1, -1, 0, 1, 1, 0); }

  virtual void ViewportDisplay(const Material *mtl) const;
};

extern Plane thePlane;

//-------------------------------------------------------------------------------

class TriObj : public Object, public cyTriMesh {
 public:
  virtual bool IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide,
                            DiffRay *diffray, DiffHitInfo *diffhit) const;

  virtual Box GetBoundBox() const
  {
    return Box(Point3(GetBoundMin().x, GetBoundMin().y, GetBoundMin().z),
               Point3(GetBoundMax().x, GetBoundMax().y, GetBoundMax().z));
  }

  virtual void ViewportDisplay(const Material *mtl) const;

  bool Load(const char *filename, bool loadMtl)
  {
    bvh.Clear();
    if (!LoadFromFileObj(filename, loadMtl)) return false;
    if (!HasNormals()) ComputeNormals();
    ComputeBoundingBox();
    bvh.SetMesh(this, 4);
    return true;
  }

 private:
  cyBVHTriMesh bvh;

  bool IntersectTriangle(const Ray &ray,
                         HitInfo &hInfo,
                         int hitSide,
                         unsigned int faceID,
                         DiffRay *diffray,
                         DiffHitInfo *diffhit) const;

  bool TraceBVHNode(const Ray &ray,
                    HitInfo &hInfo,
                    int hitSide,
                    unsigned int nodeID,
                    DiffRay *diffray,
                    DiffHitInfo *diffhit) const;
};

//-------------------------------------------------------------------------------

#endif
