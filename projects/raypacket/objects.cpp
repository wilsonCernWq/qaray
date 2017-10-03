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
#include <stack>

#define bias 0.005f

//-------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------

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
  const float entry = MAX(t0.x,MAX(t0.y,t0.z));
  const float exit  = MIN(t1.x,MIN(t1.y,t1.z));
  if (entry > t_max || entry > exit) { return false; } 
  else { return true; }
}

//-------------------------------------------------------------------------------

// Function return true only if the HitInfo has been updated!
bool Sphere::IntersectRay
(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
  // Here the ray is in model coordinate already !!!
  // Ray transformation must be done before calling this function
  const float a = glm::dot(ray.dir, ray.dir);
  const float b = 2.f * glm::dot(ray.p, ray.dir);
  const float c = glm::dot(ray.p, ray.p) - 1;
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
  const Point3 N = glm::normalize(p);
  const bool front = glm::dot(N, ray.dir) <= 0;
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

//-------------------------------------------------------------------------------

bool Plane::IntersectRay
(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
  const Point3 N(0,0,1);
  const float dz = glm::dot(ray.dir, N);
  if (ABS(dz) > 0.001f) {
    const float t = glm::dot(-ray.p, N) / dz;
    if (t <= bias) { return false; }
    const Point3 pHit = ray.p + ray.dir * t;
    if (ABS(pHit.x) <= 1.f && ABS(pHit.y) <= 1.f) {
      const Point3 p = ray.p + ray.dir * t;
      const bool front = glm::dot(N, ray.dir) <= 0;
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

//-------------------------------------------------------------------------------

bool TriObj::IntersectTriangle
(const Ray &ray, HitInfo &hInfo, int hitSide, unsigned int faceID) const
{
  auto& fidx = F(faceID);
  const auto& tmp_A = V(fidx.v[0]); //!< vertex
  const auto& tmp_B = V(fidx.v[1]); //!< vertex
  const auto& tmp_C = V(fidx.v[2]); //!< vertex

  const Point3 A(tmp_A.x, tmp_A.y, tmp_A.z); //!< vertex
  const Point3 B(tmp_B.x, tmp_B.y, tmp_B.z); //!< vertex
  const Point3 C(tmp_C.x, tmp_C.y, tmp_C.z); //!< vertex

  const Point3 N = glm::normalize(glm::cross(B-A,C-A)); //!< face normal
  //! ray - plane intersection
  const float pz = glm::dot((ray.p - A), N);
  const float dz = glm::dot(ray.dir, N);
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
  auto tmp = GetNormal(faceID, cyPoint3f(a,b,c)); // TODO
  hInfo.N = glm::normalize(Point3(tmp.x, tmp.y, tmp.z));
  hInfo.front = front;
  return true;
}

bool TriObj::IntersectRay
(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
  // ray-box intersection
  if (!GetBoundBox().IntersectRay(ray, hInfo.z)) { return false; }
  // ray-triangle intersection
  // bool hasHit = false;
  // for (unsigned int i = 0; i < NF(); ++i) {
  //   if (IntersectTriangle(ray, hInfo, hitSide, i)) { hasHit = true; }
  // }
  // return hasHit;
  return TraceBVHNode(ray, hInfo, hitSide, bvh.GetRootNodeID());
}

bool TriObj::TraceBVHNode
(const Ray &ray, HitInfo &hInfo, int hitSide, unsigned int nodeID) const
{
  //std::stack<unsigned int> localstack;
  unsigned int stack_array[40];
  unsigned int stack_idx = 0;
  
  const float threshold = 0.001f;
  const float  dx = ray.dir.x;
  const float  dy = ray.dir.y;
  const float  dz = ray.dir.z;
  const Point3  drcp = Point3(1.f, 1.f, 1.f) / ray.dir;
  const Point3& rpos = ray.p;
  
  bool hasHit = false;
  
  //localstack.push(nodeID); // initialize local stack array
  stack_array[stack_idx++] = nodeID;
  
  while (!/*localstack.empty()*/stack_idx == 0)
  {
    // get working node ID
    //const unsigned int currNodeID = localstack.top(); localstack.pop();
    const unsigned int currNodeID = stack_array[--stack_idx];
    
    if (bvh.IsLeafNode(currNodeID)) { // intersect triangle
      
      const unsigned int* triangles = bvh.GetNodeElements(currNodeID);
      for (unsigned int i = 0; i < bvh.GetNodeElementCount(currNodeID); ++i) {
        if (IntersectTriangle(ray, hInfo, hitSide, triangles[i])) {
          hasHit = true;
          
        }
      }
      
    }
    else { // traverse node
      
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
        t00.x = -BIGFLOAT; t01.x = BIGFLOAT;
        t10.x = -BIGFLOAT; t11.x = BIGFLOAT;
      }
      else {
        t00.x = MIN(p00.x, p01.x); t01.x = MAX(p00.x, p01.x);
        t10.x = MIN(p10.x, p11.x); t11.x = MAX(p10.x, p11.x);
      }
      
      if (ABS(dy) < threshold) {
        t00.y = -BIGFLOAT; t01.y = BIGFLOAT;
        t10.y = -BIGFLOAT; t11.y = BIGFLOAT;
      }
      else {
        t00.y = MIN(p00.y, p01.y); t01.y = MAX(p00.y, p01.y);
        t10.y = MIN(p10.y, p11.y); t11.y = MAX(p10.y, p11.y);
      }
      
      if (ABS(dz) < threshold) {
        t00.z = -BIGFLOAT; t01.z = BIGFLOAT;
        t10.z = -BIGFLOAT; t11.z = BIGFLOAT;
      }
      else {
        t00.z = MIN(p00.z, p01.z); t01.z = MAX(p00.z, p01.z);
        t10.z = MIN(p10.z, p11.z); t11.z = MAX(p10.z, p11.z);
      }

      const float t_max = hInfo.z;      
      const float entry0 = MAX(t00.x, MAX(t00.y, t00.z));
      const float entry1 = MAX(t10.x, MAX(t10.y, t10.z));
      const float exit0  = MIN(t01.x, MIN(t01.y, t01.z));
      const float exit1  = MIN(t11.x, MIN(t11.y, t11.z));
      const bool hasBoxHit0 = (entry0 < t_max && entry0 < exit0);
      const bool hasBoxHit1 = (entry1 < t_max && entry1 < exit1);
      
      if (hasBoxHit0 && hasBoxHit1)
      {
        if (entry0 < entry1) {
	  //localstack.push(child1); localstack.push(child0);
	  stack_array[stack_idx++] = child1;
	  stack_array[stack_idx++] = child0;
	}
	else {
	  //localstack.push(child0); localstack.push(child1);
	  stack_array[stack_idx++] = child0;
	  stack_array[stack_idx++] = child1;	    
	}
      }
      else if (hasBoxHit0 && !hasBoxHit1)
      {
        //localstack.push(child0);
	stack_array[stack_idx++] = child0;
      }
      else if (hasBoxHit1 && !hasBoxHit0)
      {
        //localstack.push(child1);
	stack_array[stack_idx++] = child1;
      }
      
    }
    
  }
  
  return hasHit;
  
}
