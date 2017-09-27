//------------------------------------------------------------------------------
///
/// \file       objects.cpp 
/// \author     Qi WU
/// \version    1.0
/// \date       August, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "objects.h"

#include <iostream>

#define bias 0.005f

// Helper functions
inline bool CheckHitSide(int hitSide, bool front) {
  return (((hitSide & ~ HIT_FRONT) && front) || 
	  ((hitSide & ~ HIT_BACK)  && !front));
}

inline float TriangleArea(size_t i, const Point3& A, const Point3& B, const Point3& C)
{
  switch (i) {
    case(0):
      return (B.y - A.y) * (C.z - A.z) - (C.y - A.y) * (B.z - A.z);
    case(1):
      return (B.x - A.x) * (C.z - A.z) - (C.x - A.x) * (B.z - A.z);
    case(2):
      return (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y);
    default:
      return TriangleArea(i % 3, A, B, C);
  }
}

// Check if ray intersects with the bounding box
bool Box::IntersectRay(const Ray &r, float t_max) const
{
  const float dx  = r.dir.x;
  const float dy  = r.dir.y;
  const float dz  = r.dir.z;
  const Point3 drcp = Point3(1.f,1.f,1.f) / r.dir;
  const Point3 p0 = -(r.p - pmin) * drcp;;
  const Point3 p1 = -(r.p - pmax) * drcp;;
  Point3 t0, t1;
  if (ABS(dx) < 0.001f) { t0.x = -BIGFLOAT; t1.x = BIGFLOAT; }
  else { 
    t0.x = MIN(p0.x, p1.x); 
    t1.x = MAX(p0.x, p1.x); 
  }
  if (ABS(dy) < 0.001f) { t0.y = -BIGFLOAT; t1.y = BIGFLOAT; }
  else { 
    t0.y = MIN(p0.y, p1.y); 
    t1.y = MAX(p0.y, p1.y); 
  }
  if (ABS(dz) < 0.001f) { t0.z = -BIGFLOAT; t1.z = BIGFLOAT; }
  else { 
    t0.z = MIN(p0.z, p1.z); 
    t1.z = MAX(p0.z, p1.z); 
  }
  if (t0.Max() > t_max || t0.Max() > t1.Min()) { return false; } 
  else { return true; }
}

// Function return true only if the HitInfo has been updated!
bool Sphere::IntersectRay
(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
  // Here the ray is in model coordinate already !!!
  // Ray transformation must be done before calling this function
  const float a = ray.dir.Dot(ray.dir);
  const float b = 2.f * ray.p.Dot(ray.dir);
  const float c = ray.p.Dot(ray.p) - 1;
  const float rcp2a = 1.f / 2.f / a;
  const float delta = b * b - 4 * a * c;
  
  // Now we need to search for a valid hit with current object
  // If there is no hit, return directly
  float t = BIGFLOAT;
  if (delta < 0) { return false; /* do nothing */ }
  else {
    if (delta == 0) { /* this section is actually useless */
      const float t0 = -b * rcp2a;
      // Select the valid root
      if (t0 <= bias) { return false; /* do nothing */ }
      else { t = t0; }
    }
    else
    {
      const float t1 = (-b - SQRT(delta)) * rcp2a;
      const float t2 = (-b + SQRT(delta)) * rcp2a;
      // Select the valid root
      if (t1 <= bias && t2 <= bias) { return false; /* do nothing */ }
      if (t1 > bias) { t = MIN(t, t1); }
      if (t2 > bias) { t = MIN(t, t2); }
    }
  }
  
  // If the code reaches here, it means we must have found a valid hit
  // with the current object. Now we have to compare the hit we found
  // with the previous hit
  // We select the smaller root and compare it with previous hit
  // update hit info if the previous hit is behind the current hit
  const Point3 p = ray.p + ray.dir * t;
  const Point3 N = p.GetNormalized();
  const bool front = N.Dot(ray.dir) <= 0;
  if (CheckHitSide(hitSide, front))
  {
    if (hInfo.z > t) {
      hInfo.z = t;
      hInfo.p = p;
      hInfo.N = N;
      hInfo.front = front;
      return true;
    }
  }
  return false; /* do nothing */
}

bool Plane::IntersectRay
(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
  const Point3 N(0,0,1);
  const float dz = ray.dir.Dot(N);
  if (ABS(dz) > 0.001f) {
    const float t = -ray.p.Dot(N) / dz;
    if (t <= bias) { return false; }
    const Point3 pHit = ray.p + ray.dir * t;
    if (ABS(pHit.x) <= 1.f && ABS(pHit.y) <= 1.f) {
      const Point3 p = ray.p + ray.dir * t;
      const bool front = N.Dot(ray.dir) <= 0;
      if (CheckHitSide(hitSide, front))
      {
        if (hInfo.z > t) {
          hInfo.z = t;
          hInfo.p = p;
          hInfo.N = N;
          hInfo.front = front;
          return true;
        }
      }
    }
  }
  return false;
}

bool TriObj::IntersectTriangle(const Ray &ray, HitInfo &hInfo, int hitSide, unsigned int faceID) const
{
  auto& fidx = F(faceID);
  const Point3 A = V(fidx.v[0]); //!< vertex
  const Point3 B = V(fidx.v[1]); //!< vertex
  const Point3 C = V(fidx.v[2]); //!< vertex
  const Point3 N = (B-A).Cross(C-A).GetNormalized(); //!< face normal
  //! ray - plane intersection
  const float pz = (ray.p - A).Dot(N);
  const float dz = ray.dir.Dot(N);
  if (ABS(dz) < 0.001f) { return false; }
  const float t = -pz / dz;
  if (t <= bias) { return false; }
  const bool front = dz <= 0; //!< roughly check it is a front or back hit
  if (!CheckHitSide(hitSide, front)) { return false; }
  if (hInfo.z <= t) { return false; /* This is not an interesting triangle */ }
  //! project triangle onto 2D plane
  //! compute barycentric coordinate
  const Point3 p = ray.p + t * ray.dir;
  size_t ignoredAxis;
  if (ABS(N.x) > ABS(N.y) && ABS(N.x) > ABS(N.z)) { ignoredAxis = 0; }
  else if (ABS(N.y) > ABS(N.z)) { ignoredAxis = 1; }
  else { ignoredAxis = 2; }
  const float s = 1.f / TriangleArea(ignoredAxis, A, B, C);
  const float a = TriangleArea(ignoredAxis, p, B, C) * s;
  const float b = TriangleArea(ignoredAxis, p, C, A) * s;
  const float c = 1.f - a - b;
  if (a < 0 || b < 0 || c < 0) { return false; }
  //! now we have a hit with this triangle
  hInfo.z = t;
  hInfo.p = p;
  hInfo.N = GetNormal(faceID, Point3(a,b,c));
  hInfo.front = front;
  return true;
}

bool TriObj::IntersectRay
(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
  // ray-box intersection
  if (!GetBoundBox().IntersectRay(ray, hInfo.z)) { return false; }
  // ray-triangle intersection
  bool hasHit = false;
  for (unsigned int i = 0; i < NF(); ++i) {
    if (IntersectTriangle(ray, hInfo, hitSide, i)) { hasHit = true; }
  }
  return hasHit;
}
