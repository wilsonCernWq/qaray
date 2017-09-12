//------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.1
/// \date       August 29, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Qi WU
///
/// \brief Modified for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <atomic>

#include "lodepng.h"

/* #include "cyPoint.h" */
/* typedef cyPoint2f Point2; */
/* typedef cyPoint3f Point3; */
/* typedef cyPoint4f Point4; */

/* #include "cyMatrix.h" */
/* typedef cyMatrix3f Matrix3; */

/* #include "cyColor.h" */
/* typedef cyColor Color; */
/* typedef cyColorA ColorA; */
/* typedef cyColor24 Color24; */

typedef unsigned char uchar;

//------------------------------------------------------------------------------

#ifndef min
# define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
# define max(a,b) ((a)>(b)?(a):(b))
#endif

#define BIGFLOAT 1.0e30f

//------------------------------------------------------------------------------

class Ray
{
public:
  Point3 p, dir;

  Ray() {}
Ray(const Point3 &_p, const Point3 &_dir) : p(_p), dir(_dir) {}
Ray(const Ray &r) : p(r.p), dir(r.dir) {}
  void Normalize() { dir.Normalize(); }
};

//------------------------------------------------------------------------------

class Node;

#define HIT_NONE			0
#define HIT_FRONT			1
#define HIT_BACK			2
#define HIT_FRONT_AND_BACK	(HIT_FRONT|HIT_BACK)

struct HitInfo
{
  float z;			// the distance from the ray center to the hit point
  Point3 p;			// position of the hit point
  Point3 N;			// surface normal at the hit point
  const Node *node;	// the object node that was hit
  bool front;			// true if the ray hits the front side, false if the ray hits the back side
  int mtlID;			// sub-material index

  HitInfo() { Init(); }
  void Init() { z=BIGFLOAT; node=NULL; front=true; mtlID=0; }
};


//------------------------------------------------------------------------------

class Transformation
{
private:
  Matrix3 tm;						// Transformation matrix to the local space
  Point3 pos;						// Translation part of the transformation matrix
  mutable Matrix3 itm;			// Inverse of the transformation matrix (cached)
public:
Transformation() : pos(0,0,0) { tm.SetIdentity(); itm.SetIdentity(); }
  const Matrix3& GetTransform() const { return tm; }
  const Point3& GetPosition() const { return pos; }
  const Matrix3&	GetInverseTransform() const { return itm; }

  Point3 TransformTo( const Point3 &p ) const { return itm * (p - pos); }	// Transform to the local coordinate system
  Point3 TransformFrom( const Point3 &p ) const { return tm*p + pos; }	// Transform from the local coordinate system

  // Transforms a vector to the local coordinate system (same as multiplication with the inverse transpose of the transformation)
  Point3 VectorTransformTo( const Point3 &dir ) const { return TransposeMult(tm,dir); }

  // Transforms a vector from the local coordinate system (same as multiplication with the inverse transpose of the transformation)
  Point3 VectorTransformFrom( const Point3 &dir ) const { return TransposeMult(itm,dir); }

  void Translate(Point3 p) { pos+=p; }
  void Rotate(Point3 axis, float degree) { Matrix3 m; m.SetRotation(axis,degree*(float)M_PI/180.0f); Transform(m); }
  void Scale(float sx, float sy, float sz) { Matrix3 m; m.Zero(); m[0]=sx; m[4]=sy; m[8]=sz; Transform(m); }
  void Transform(const Matrix3 &m) { tm=m*tm; pos=m*pos; tm.GetInverse(itm); }

  void InitTransform() { pos.Zero(); tm.SetIdentity(); itm.SetIdentity(); }

private:
  // Multiplies the given vector with the transpose of the given matrix
  static Point3 TransposeMult( const Matrix3 &m, const Point3 &dir )
  {
    Point3 d;
    d.x = m.GetColumn(0) % dir;
    d.y = m.GetColumn(1) % dir;
    d.z = m.GetColumn(2) % dir;
    return d;
  }
};

//------------------------------------------------------------------------------

class Material;

// Base class for all object types
class Object
{
public:
  virtual bool IntersectRay( const Ray &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const=0;
  virtual void ViewportDisplay(const Material *mtl) const {}	// used for OpenGL display
};

typedef ItemFileList<Object> ObjFileList;

//------------------------------------------------------------------------------

class Light : public ItemBase
{
public:
  virtual Color	Illuminate(const Point3 &p, const Point3 &N) const=0;
  virtual Point3	Direction (const Point3 &p) const=0;
  virtual bool	IsAmbient () const { return false; }
  virtual void	SetViewportLight(int lightID) const {}	// used for OpenGL display
};

class LightList : public ItemList<Light> {};

//------------------------------------------------------------------------------

class Material : public ItemBase
{
public:
  // The main method that handles the shading by calling all the lights in the list.
  // ray: incoming ray,
  // hInfo: hit information for the point that is being shaded, lights: the light list,
  virtual Color Shade(const Ray &ray, const HitInfo &hInfo, const LightList &lights) const=0;

  virtual void SetViewportMaterial(int subMtlID=0) const {}	// used for OpenGL display
};

class MaterialList : public ItemList<Material>
{
public:
  Material* Find( const char *name ) { int n=size(); for ( int i=0; i<n; i++ ) if ( at(i) && strcmp(name,at(i)->GetName())==0 ) return at(i); return NULL; }
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

class Camera
{
public:
  Point3 pos, dir, up;
  float fov;
  int imgWidth, imgHeight;

  void Init()
  {
    pos.Set(0,0,0);
    dir.Set(0,0,-1);
    up.Set(0,1,0);
    fov = 40;
    imgWidth = 200;
    imgHeight = 150;
  }
};

