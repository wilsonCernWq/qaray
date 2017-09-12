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
/// \file       transform.h 
/// \author     Qi WU
///
/// \brief Modified Transformation class for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "math/math.h"

namespace qw {

  class Transformation {
  private:
    // normal version
    vec3f pos; // Translation part of the transformation matrix
    mat3f tm;  // Transformation matrix from the local space to the world space
    mutable mat3f itm; // Inverse of the transformation matrix (input matrix)
    // SIMD version
    vec3fv packet_pos;
    mat3fv packet_tm;
    mat3fv packet_itm;
  public:
    Transformation();
    const mat3f& GetTransform() const;
    const vec3f& GetPosition()  const;
    const mat3f& GetInverseTransform() const;

    //! SIMD Calls
    // Transform to the local coordinate system
    vec3fv PointTransformInTo(const vec3fv &p) const;
    // Transform from the local coordinate system
    vec3fv PointTransformFrom(const vec3fv &p) const;
    // Transforms a vector to the local coordinate system 
    // (same as multiplication with the inverse transpose of the transformation)
    vec3fv VectorTransformInTo(const vec3fv &v) const;
    // Transforms a vector from the local coordinate system 
    // (same as multiplication with the inverse transpose of the transformation)
    vec3fv VectorTransformFrom(const vec3fv &v) const;
    //! Normal Calls
    void Translate(const vec3f& p);
    void Rotate(const vec3f& axis, const float degree);
    void Scale(const float sx, const float sy, const float sz);
    void Transform(const mat3f& m);
    void InitTransform();
    void FinalizeTransform();

private:
    // Multiplies the given vector with the transpose of the given matrix
    static vec3fv TransposeMult(const mat3fv &m, const vec3fv &v);
  };

};
