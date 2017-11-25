///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/23/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.             //
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

#include "box.h"
#include "core/ray.h"

namespace qaray {
Box::Box(const Point3 &pos_min, const Point3 &pos_max) :
    pmin(pos_min),
    pmax(pos_max)
{}
Box::Box(float xmin, float ymin, float zmin,
         float xmax, float ymax, float zmax) :
    pmin(xmin, ymin, zmin),
    pmax(xmax, ymax, zmax)
{}
Box::Box(const float *dim) :
    pmin(dim[0], dim[1], dim[2]),
    pmax(dim[3], dim[4], dim[5])
{}
// Initializes the box, such that there exists no point inside the box (i.e. it is empty).
void Box::Init()
{
  pmin = Point3(BIGFLOAT, BIGFLOAT, BIGFLOAT);
  pmax = Point3(-BIGFLOAT, -BIGFLOAT, -BIGFLOAT);
}
// Returns true if the box is empty; otherwise, returns false.
bool Box::IsEmpty() const
{
  return pmin.x > pmax.x || pmin.y > pmax.y || pmin.z > pmax.z;
}
// Returns one of the 8 corner point of the box in the following order:
// 0:(x_min,y_min,z_min), 1:(x_max,y_min,z_min)
// 2:(x_min,y_max,z_min), 3:(x_max,y_max,z_min)
// 4:(x_min,y_min,z_max), 5:(x_max,y_min,z_max)
// 6:(x_min,y_max,z_max), 7:(x_max,y_max,z_max)
Point3 Box::Corner(int i) const  // 8 corners of the box
{
  Point3 p;
  p.x = (i & 1) ? pmax.x : pmin.x;
  p.y = (i & 2) ? pmax.y : pmin.y;
  p.z = (i & 4) ? pmax.z : pmin.z;
  return p;
}
// Enlarges the box such that it includes the given point p.
void Box::operator+=(const Point3 &p)
{
  for (int i = 0; i < 3; i++) {
    if (pmin[i] > p[i]) pmin[i] = p[i];
    if (pmax[i] < p[i]) pmax[i] = p[i];
  }
}

// Enlarges the box such that it includes the given box b.
void Box::operator+=(const Box &b)
{
  for (int i = 0; i < 3; i++) {
    if (pmin[i] > b.pmin[i]) pmin[i] = b.pmin[i];
    if (pmax[i] < b.pmax[i]) pmax[i] = b.pmax[i];
  }
}
// Returns true if the point is inside the box; otherwise, returns false.
bool Box::IsInside(const Point3 &p) const
{
  for (int i = 0; i < 3; i++)
    if (pmin[i] > p[i] || pmax[i] < p[i]) return false;
  return true;
}
// Check If Ray Intersects with the Bounding Box
bool Box::IntersectRay(const Ray &r, float t_max) const
{
  const float dx = r.dir.x;
  const float dy = r.dir.y;
  const float dz = r.dir.z;
  const Point3 drcp = Point3(1.f, 1.f, 1.f) / r.dir;
  const Point3 p0 = -(r.p - pmin) * drcp;;
  const Point3 p1 = -(r.p - pmax) * drcp;;
  Point3 t0, t1;
  if (ABS(dx) < 1e-7f) {
    t0.x = -BIGFLOAT;
    t1.x = BIGFLOAT;
  } else {
    t0.x = MIN(p0.x, p1.x);
    t1.x = MAX(p0.x, p1.x);
  }
  if (ABS(dy) < 1e-7f) {
    t0.y = -BIGFLOAT;
    t1.y = BIGFLOAT;
  } else {
    t0.y = MIN(p0.y, p1.y);
    t1.y = MAX(p0.y, p1.y);
  }
  if (ABS(dz) < 1e-7f) {
    t0.z = -BIGFLOAT;
    t1.z = BIGFLOAT;
  } else {
    t0.z = MIN(p0.z, p1.z);
    t1.z = MAX(p0.z, p1.z);
  }
  const float entry = MAX(t0.x, MAX(t0.y, t0.z));
  const float exit  = MIN(t1.x, MIN(t1.y, t1.z));
  if (entry > t_max || entry > exit) { return false; }
  else { return true; }
}
}