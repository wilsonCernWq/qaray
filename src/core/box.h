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

#ifndef QARAY_BOX_H
#define QARAY_BOX_H
#pragma once

#include "core/setup.h"
#include "math/math.h"

namespace qaray {
class Box {
 public:
  Point3 pmin, pmax;
 public:
  Box() { Init(); }
  ~Box() = default;
  Box(const Point3 &, const Point3 &);
  Box(float, float, float, float, float, float);
  explicit Box(const float *);
  // Initializes the box, such that there exists no point inside the box (i.e. it is empty).
  void Init();
  // Returns true if the box is empty; otherwise, returns false.
  bool IsEmpty() const;
  // Returns one of the 8 corner point of the box in the following order:
  // 0:(x_min,y_min,z_min), 1:(x_max,y_min,z_min)
  // 2:(x_min,y_max,z_min), 3:(x_max,y_max,z_min)
  // 4:(x_min,y_min,z_max), 5:(x_max,y_min,z_max)
  // 6:(x_min,y_max,z_max), 7:(x_max,y_max,z_max)
  // 8 corners of the box
  Point3 Corner(int i) const;
  // Enlarges the box such that it includes the given point p.
  void operator+=(const Point3 &p);
  // Enlarges the box such that it includes the given box b.
  void operator+=(const Box &b);
  // Returns true if the point is inside the box; otherwise, returns false.
  bool IsInside(const Point3 &p) const;
  // Returns true if the ray intersects with the box for any parameter that
  // is smaller than t_max; otherwise, returns false.
  bool IntersectRay(const Ray &r, float t_max) const;
};
}

#endif //QARAY_BOX_H
