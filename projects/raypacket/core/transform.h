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

#ifndef QARAY_TRANSFORM_H
#define QARAY_TRANSFORM_H
#pragma once

#include "core/core.h"
#include "math/math.h"

namespace qaray {
class Transformation {
 private:
  Point3 pos;          // Translation part of the transformation matrix
  Matrix3 tm;          // Transformation matrix to the local space
  mutable Matrix3 itm; // Inverse of the transformation matrix (cached)
 public:
  Transformation();
  const Point3  &GetPosition() const { return pos; }
  const Matrix3 &GetTransform() const { return tm; }
  const Matrix3 &GetInverseTransform() const { return itm; }
  // Transform to the local coordinate system
  Point3 TransformTo(const Point3 &p) const { return itm * (p - pos); }
  // Transform from the local coordinate system
  Point3 TransformFrom(const Point3 &p) const { return tm * p + pos; }
  // Transforms a vector to the local coordinate system
  // (same as multiplication with the inverse transpose of the transformation)
  Point3 VectorTransformTo(const Point3 &dir) const
  {
    return TransposeMult(tm, dir);
  }
  // Transforms a vector from the local coordinate system
  // (same as multiplication with the inverse transpose of the transformation)
  Point3 VectorTransformFrom(const Point3 &dir) const
  {
    return TransposeMult(itm, dir);
  }
  // Rigid motions
  void Translate(Point3 p) { pos += p; }
  void Rotate(Point3 axis, float degree)
  {
    Matrix3 m(glm::rotate(Matrix4(1.0f), degree * PI / 180.0f, axis));
    Transform(m);
  }
  void Scale(float sx, float sy, float sz)
  {
    Matrix3 m(glm::scale(Matrix4(1.0f), Point3(sx, sy, sz)));
    Transform(m);
  }
  void Transform(const Matrix3 &m);
  void InitTransform();
 private:
  // Multiplies the given vector with the transpose of the given matrix
  static Point3 TransposeMult(const Matrix3 &m, const Point3 &dir);
};
}

#endif //QARAY_TRANSFORM_H
