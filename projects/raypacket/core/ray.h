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

#ifndef QARAY_RAY_H
#define QARAY_RAY_H
#pragma once

#include "core/setup.h"
#include "math/math.h"

namespace qaray {
class Ray {
 public:
  Point3 p, dir;
  Ray() = default;
  Ray(const Ray &r) = default;
  Ray(const Point3 &p, const Point3 &d) : p(p), dir(d) {}
  void Normalize() { dir = glm::normalize(dir); }
};
class DiffRay {
 public:
  static const float dx, dy, rdx, rdy;
  Ray c, x, y;
  bool hasDiffRay = true;
 public:
  DiffRay() = default;
  DiffRay(const Point3 &p, const Point3 &d) :
      c(p, d),
      x(p, d),
      y(p, d),
      hasDiffRay(false)
  {}
  DiffRay(const Point3 &pc, const Point3 &dc,
          const Point3 &px, const Point3 &dx,
          const Point3 &py, const Point3 &dy) :
      c(pc, dc),
      x(px, dx),
      y(py, dy),
      hasDiffRay(true)
  {}
  DiffRay(const DiffRay &r) = default;
  void Normalize()
  {
    c.Normalize();
    x.Normalize();
    y.Normalize();
  }
};
}

#endif //QARAY_RAY_H
