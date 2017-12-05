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
#include <stack>

Sphere theSphere;
Plane thePlane;

#define bias 0.005f
//-------------------------------------------------------------------------------

// Helper functions
inline bool CheckHitSide(const int &hitSide, const bool &front)
{
  return
      (((hitSide & ~(HIT_FRONT)) && front) ||
      ((hitSide & ~(HIT_BACK))   && !front));
}

inline float TriangleArea(size_t i,
                          const Point3 &A,
                          const Point3 &B,
                          const Point3 &C)
{
  switch (i) {
    case (0):return (B.y - A.y) * (C.z - A.z) - (C.y - A.y) * (B.z - A.z);
    case (1):return (B.x - A.x) * (C.z - A.z) - (C.x - A.x) * (B.z - A.z);
    case (2):return (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y);
    default:return TriangleArea(i % 3, A, B, C);
  }
}

//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// Function return true only if the HitInfo has been updated!
inline Point3 Sphere_TexCoord(const Point3 &p, const float rcp_l = 1.f)
{
  return Point3(0.5f - atan2(p.x, p.y) * RCP_2PI,
                0.5f + asin(p.z * rcp_l) * RCP_PI,
                0.f);
}

bool Sphere::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide,
                          DiffRay *diffray, DiffHitInfo *diffhit) const
{
  // Here the ray is in model coordinate already !!!
  // Ray transformation must be done before calling this function
  const float a = dot(ray.dir, ray.dir);
  const float b = 2.f * dot(ray.p, ray.dir);
  const float c = dot(ray.p, ray.p) - 1;
  const float rcp2a = 1.f / (2.f * a);
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
    } else {
      const float sqrt_delta = SQRT(delta);
      const float t1 = (-b - sqrt_delta) * rcp2a;
      const float t2 = (-b + sqrt_delta) * rcp2a;
      // Select the valid root
      if (t1 <= bias && t2 <= bias) { return false; /* do nothing */ }
      else if (t1 > bias) { t = MIN(t, t1); }
      else if (t2 > bias) { t = MIN(t, t2); }
    }
  }

  // If the code reaches here, it means we must have found a valid hit
  // with the current object. Now we have to compare the hit we found
  // with the previous hit
  // We select the smaller root and compare it with previous hit
  // update hit info if the previous hit is behind the current hit
  if (hInfo.z > t) {
    const Point3 p = ray.p + ray.dir * t;
    const Point3 N = normalize(p);
    const bool front = (dot(N, ray.dir) <= 0);
    if (CheckHitSide(hitSide, front)) {
      hInfo.z = t;
      // non shadow ray
      if (diffray != NULL && diffhit != NULL) {
        hInfo.p = p;
        hInfo.N = N;
        hInfo.hasFrontHit = front;
        // Texture Coordinate at the Hit Point
        hInfo.hasTexture = true;
        hInfo.uvw = Sphere_TexCoord(p);
        // Differential Rays
        if (diffray->hasDiffRay) {
          const float pz_x = dot((diffray->x.p - p), N);
          const float pz_y = dot((diffray->y.p - p), N);
          const float dz_x = dot(diffray->x.dir, N);
          const float dz_y = dot(diffray->y.dir, N);
          const float t_x = -pz_x / dz_x;
          const float t_y = -pz_y / dz_y;
          const Point3 p_x = diffray->x.p + diffray->x.dir * t_x;
          const Point3 p_y = diffray->y.p + diffray->y.dir * t_y;
          diffhit->x.z = t_x;
          diffhit->x.p = p_x;
          diffhit->x.N = glm::normalize(p_x);
          diffhit->y.z = t_y;
          diffhit->y.p = p_y;
          diffhit->y.N = glm::normalize(p_y);
          hInfo.duvw[0] = DiffRay::rdx
              * (Sphere_TexCoord(p_x, 1.f / length(p_x)) - hInfo.uvw);
          hInfo.duvw[1] = DiffRay::rdy
              * (Sphere_TexCoord(p_y, 1.f / length(p_y)) - hInfo.uvw);
        } else {
          diffhit->x.z = t;
          diffhit->x.p = p;
          diffhit->x.N = N;
          diffhit->y.z = t;
          diffhit->y.p = p;
          diffhit->y.N = N;
          hInfo.duvw[0] = Point3(0.f);
          hInfo.duvw[1] = Point3(0.f);
        }
      }
      return true;
    }
  }
  return false; /* do nothing */
}

//-------------------------------------------------------------------------------
inline Point3 Plane_TexCoord(const Point3 &p)
{
  return Point3((p.x + 1.f) * 0.5f, (p.y + 1.f) * 0.5f, 0.f);
}

bool Plane::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide,
                         DiffRay *diffray, DiffHitInfo *diffhit) const
{
  static const Point3 N(0, 0, 1);
  const float dz = dot(ray.dir, N);
  if (ABS(dz) < 1e-7f) { return false; /* ray parallel to plane */}
  const float pz = dot(ray.p, N);
  const float t = -pz / dz;
  if (t <= bias) { return false; /* hit is too closed to the previous hit */}
  if (hInfo.z > t) {
    // Continue Only If This Hit Is Potentially Closer !!!
    const Point3 p = ray.p + ray.dir * t;
    if (ABS(p.x) > 1.f || ABS(p.y) > 1.f) { return false; }
    const bool front = (dot(N, ray.dir) <= 0);
    if (CheckHitSide(hitSide, front)) {
      hInfo.z = t;
      // Non-Shadow Ray
      if (diffray != NULL && diffhit != NULL) {
        hInfo.p = p;
        hInfo.N = N;
        hInfo.hasFrontHit = front;
        // texture coordinates
        hInfo.hasTexture = true;
        hInfo.uvw = Plane_TexCoord(p);
        // Differential Rays
        if (diffray->hasDiffRay) {
          const float pz_x = dot(diffray->x.p, N);
          const float pz_y = dot(diffray->y.p, N);
          const float dz_x = dot(diffray->x.dir, N);
          const float dz_y = dot(diffray->y.dir, N);
          const float t_x = -pz_x / dz_x;
          const float t_y = -pz_y / dz_y;
          const Point3 p_x = diffray->x.p + diffray->x.dir * t_x;
          const Point3 p_y = diffray->y.p + diffray->y.dir * t_y;
          diffhit->x.z = t_x;
          diffhit->x.p = p_x;
          diffhit->x.N = N;
          diffhit->y.z = t_y;
          diffhit->y.p = p_y;
          diffhit->y.N = N;
          hInfo.duvw[0] =
              DiffRay::rdx * (Plane_TexCoord(p_x) - hInfo.uvw);
          hInfo.duvw[1] =
              DiffRay::rdy * (Plane_TexCoord(p_y) - hInfo.uvw);
        } else {
          diffhit->x.z = t;
          diffhit->x.p = p;
          diffhit->x.N = N;
          diffhit->y.z = t;
          diffhit->y.p = p;
          diffhit->y.N = N;
          hInfo.duvw[0] = Point3(0.f);
          hInfo.duvw[1] = Point3(0.f);
        }
      }
      return true;
    }
  }
  return false;
}

//-------------------------------------------------------------------------------

bool TriObj::IntersectTriangle(const Ray &ray, HitInfo &hInfo,
                               int hitSide, unsigned int faceID,
                               DiffRay *diffray, DiffHitInfo *diffhit) const
{
  auto &fidx = F(faceID);
  const auto &tmp_A = V(fidx.v[0]); //!< vertex
  const auto &tmp_B = V(fidx.v[1]); //!< vertex
  const auto &tmp_C = V(fidx.v[2]); //!< vertex
  const Point3 A = Point3(tmp_A.x, tmp_A.y, tmp_A.z); //!< vertex
  const Point3 B = Point3(tmp_B.x, tmp_B.y, tmp_B.z); //!< vertex
  const Point3 C = Point3(tmp_C.x, tmp_C.y, tmp_C.z); //!< vertex
  const Point3
      N = normalize(cross((B - A), (C - A))); //!< face normal
  //! ray - plane intersection
  const float dz = dot(ray.dir, N);
  if (ABS(dz) < 1e-7f) { return false; /* ray parallel to plane */}
  const float pz = dot((ray.p - A), N);
  const float t = -pz / dz;
  if (t
      <= bias) { return false; /* intersect is too closed to the previous hit */ }
  if (hInfo.z > t) {
    const bool front = (dz <= 0); //!< roughly check it is a front or back hit
    if (CheckHitSide(hitSide, front)) {
      // Project Triangle onto 2D Plane
      // Compute Barycentric Coordinate
      const Point3 p = ray.p + t * ray.dir;
      size_t ignoredAxis;
      const float abs_nx = ABS(N.x);
      const float abs_ny = ABS(N.y);
      const float abs_nz = ABS(N.z);
      if (abs_nx > abs_ny && abs_nx > abs_nz) { ignoredAxis = 0; }
      else if (abs_ny > abs_nz) { ignoredAxis = 1; }
      else { ignoredAxis = 2; }
      const float s = 1.f / TriangleArea(ignoredAxis, A, B, C);
      const float a = TriangleArea(ignoredAxis, p, B, C) * s;
      const float b = TriangleArea(ignoredAxis, p, C, A) * s;
      const float c = 1.f - a - b;
      if (a < 0 || b < 0
          || c < 0) { return false; /* hit not inside the triangle */}
      // Now We Have a Hit with This Triangle
      const Point3 bc(a, b, c);
      hInfo.z = t;
      // Non-Shadow Ray
      if (diffray != NULL && diffhit != NULL) {
        const auto tmp_N = GetNormal(faceID, cyPoint3f(bc.x, bc.y, bc.z));
        hInfo.p = p;
        hInfo.N = Point3(tmp_N.x, tmp_N.y, tmp_N.z);
        hInfo.hasFrontHit = front;
        hInfo.mtlID = GetMaterialIndex(faceID);
        // Texture Coordinates
        // TODO: we need to remove cyCodeBase dependencies
        cyPoint3f tmp_uvw, tmp_duvw0, tmp_duvw1;
        if (HasTextureVertices()) {
          hInfo.hasTexture = true;
          tmp_uvw = GetTexCoord(faceID, cyPoint3f(bc.x, bc.y, bc.z));
          hInfo.uvw = Point3(tmp_uvw.x, tmp_uvw.y, tmp_uvw.z);
        }
        // Ray Differential
        if (diffray->hasDiffRay) {
          const float pz_x = dot((diffray->x.p - A), N);
          const float pz_y = dot((diffray->y.p - A), N);
          const float dz_x = dot(diffray->x.dir, N);
          const float dz_y = dot(diffray->y.dir, N);
          const float t_x = -pz_x / dz_x;
          const float t_y = -pz_y / dz_y;
          const Point3 p_x = diffray->x.p + diffray->x.dir * t_x;
          const Point3 p_y = diffray->y.p + diffray->y.dir * t_y;
          const float ax = TriangleArea(ignoredAxis, p_x, B, C) * s;
          const float bx = TriangleArea(ignoredAxis, p_x, C, A) * s;
          const float cx = 1.f - ax - bx;
          const float ay = TriangleArea(ignoredAxis, p_y, B, C) * s;
          const float by = TriangleArea(ignoredAxis, p_y, C, A) * s;
          const float cy = 1.f - ay - by;
          diffhit->x.z = t_x;
          diffhit->x.p = p_x;
          diffhit->x.N = hInfo.N;
          diffhit->y.z = t_y;
          diffhit->y.p = p_y;
          diffhit->y.N = hInfo.N;
          if (HasTextureVertices()) {
            tmp_duvw0 = DiffRay::rdx
                * (GetTexCoord(faceID, cyPoint3f(ax, bx, cx)) - tmp_uvw);
            tmp_duvw1 = DiffRay::rdy
                * (GetTexCoord(faceID, cyPoint3f(ay, by, cy)) - tmp_uvw);
            hInfo.duvw[0] = Point3(tmp_duvw0.x, tmp_duvw0.y, tmp_duvw0.z);
            hInfo.duvw[1] = Point3(tmp_duvw1.x, tmp_duvw1.y, tmp_duvw1.z);
          }
        } else {
          diffhit->x.z = t;
          diffhit->x.p = p;
          diffhit->x.N = hInfo.N;
          diffhit->y.z = t;
          diffhit->y.p = p;
          diffhit->y.N = hInfo.N;
          hInfo.duvw[0] = Point3(0.f);
          hInfo.duvw[1] = Point3(0.f);
        }
      }
      return true;
    }
  }
  return false;
}

//-------------------------------------------------------------------------------

bool TriObj::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide,
                          DiffRay *diffray, DiffHitInfo *diffhit) const
{
  // ray-box intersection
  if (!GetBoundBox().IntersectRay(ray, hInfo.z)) { return false; }
  // ray-triangle intersection
  return TraceBVHNode(ray,
                      hInfo,
                      hitSide,
                      bvh.GetRootNodeID(),
                      diffray,
                      diffhit);
}

bool TriObj::TraceBVHNode(const Ray &ray,
                          HitInfo &hInfo,
                          int hitSide,
                          unsigned int nodeID,
                          DiffRay *diffray,
                          DiffHitInfo *diffhit) const
{
  unsigned int stack_array[40];
  unsigned int stack_idx = 0;
  const float threshold = 1e-7f;
  const float dx = ray.dir.x;
  const float dy = ray.dir.y;
  const float dz = ray.dir.z;
  const Point3 drcp = Point3(1.f, 1.f, 1.f) / ray.dir;
  const Point3 &rpos = ray.p;
  bool hasHit = false;
  // initialize local stack array
  stack_array[stack_idx++] = nodeID;
  while (stack_idx != 0) {
    // get working node ID
    const unsigned int currNodeID = stack_array[--stack_idx];
    if (bvh.IsLeafNode(currNodeID)) { // intersect triangle
      const unsigned int *triangles = bvh.GetNodeElements(currNodeID);
      for (unsigned int i = 0; i < bvh.GetNodeElementCount(currNodeID); ++i) {
        if (IntersectTriangle(ray, hInfo, hitSide, triangles[i],
                              diffray, diffhit)) { hasHit = true; }
      }
    } else { // traverse node
      // get two children
      unsigned int child0 = 0, child1 = 0;
      bvh.GetChildNodes(currNodeID, child0, child1);
      // get two boxes
      Box box0(bvh.GetNodeBounds(child0));
      Box box1(bvh.GetNodeBounds(child1));
      // manually intersect with two boxes
      const Point3 p00 = -(rpos - box0.pmin) * drcp;
      const Point3 p01 = -(rpos - box0.pmax) * drcp;
      const Point3 p10 = -(rpos - box1.pmin) * drcp;
      const Point3 p11 = -(rpos - box1.pmax) * drcp;
      Point3 t00, t01, t10, t11;
      if (ABS(dx) < threshold) {
        t00.x = -BIGFLOAT;
        t01.x = BIGFLOAT;
        t10.x = -BIGFLOAT;
        t11.x = BIGFLOAT;
      } else {
        t00.x = MIN(p00.x, p01.x);
        t01.x = MAX(p00.x, p01.x);
        t10.x = MIN(p10.x, p11.x);
        t11.x = MAX(p10.x, p11.x);
      }
      if (ABS(dy) < threshold) {
        t00.y = -BIGFLOAT;
        t01.y = BIGFLOAT;
        t10.y = -BIGFLOAT;
        t11.y = BIGFLOAT;
      } else {
        t00.y = MIN(p00.y, p01.y);
        t01.y = MAX(p00.y, p01.y);
        t10.y = MIN(p10.y, p11.y);
        t11.y = MAX(p10.y, p11.y);
      }
      if (ABS(dz) < threshold) {
        t00.z = -BIGFLOAT;
        t01.z = BIGFLOAT;
        t10.z = -BIGFLOAT;
        t11.z = BIGFLOAT;
      } else {
        t00.z = MIN(p00.z, p01.z);
        t01.z = MAX(p00.z, p01.z);
        t10.z = MIN(p10.z, p11.z);
        t11.z = MAX(p10.z, p11.z);
      }
      const float t_max = hInfo.z;
      const float entry0 = MAX(t00.x, MAX(t00.y, t00.z));
      const float entry1 = MAX(t10.x, MAX(t10.y, t10.z));
      const float exit0 = MIN(t01.x, MIN(t01.y, t01.z));
      const float exit1 = MIN(t11.x, MIN(t11.y, t11.z));
      const bool hasBoxHit0 = (entry0 < t_max && entry0 < exit0);
      const bool hasBoxHit1 = (entry1 < t_max && entry1 < exit1);
      if (hasBoxHit0 && hasBoxHit1) {
        if (entry0 < entry1) {
          stack_array[stack_idx++] = child1;
          stack_array[stack_idx++] = child0;
        } else {
          stack_array[stack_idx++] = child0;
          stack_array[stack_idx++] = child1;
        }
      } else if (hasBoxHit0 && !hasBoxHit1) {
        stack_array[stack_idx++] = child0;
      } else if (hasBoxHit1 && !hasBoxHit0) {
        stack_array[stack_idx++] = child1;
      }
    }
  }
  return hasHit;
}
