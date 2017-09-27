#include "sphere.h"

namespace qw {
// Helper functions
inline bool CheckHitSide(int hitSide, bool front) {
  return (((hitSide & ~ HIT_FRONT) && front) || 
	  ((hitSide & ~ HIT_BACK)  && !front));
}

inline float TriangleArea(size_t i, const vec3fv& A, const vec3fv& B, const vec3fv& C)
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

// Function return true only if the HitInfo has been updated!
bool Sphere::IntersectRay
(const RayPacket& ray, HitInfo &hInfo, int hitSide) const
{
  // Here the ray is in model coordinate already !!!
  // Ray transformation must be done before calling this function
  const vfloat a = glm::dot(ray.dir, ray.dir);
  const vfloat b = 2.f * ray.p.Dot(ray.dir);
  const vfloat c = ray.p.Dot(ray.p) - 1;
  const vfloat rcp2a = 1.f / 2.f / a;
  const vfloat delta = b * b - 4 * a * c;
  
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
};
