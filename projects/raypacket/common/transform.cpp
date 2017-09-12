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

#include "transform.h"

namespace qw 
{
  Transformation::Transformation() : pos(0.f), tm(1.f), itm(1.f) {}
  const mat3f& Transformation::GetTransform() const { return  tm; }
  const vec3f& Transformation::GetPosition()  const { return pos; }
  const mat3f& Transformation::GetInverseTransform() const { return itm; }

  //! SIMD Calls
  // Transform to the local coordinate system
  vec3fv Transformation::PointTransformInTo(const vec3fv &p) const 
  { 
    return packet_itm * (p - packet_pos); 
  }
  // Transform from the local coordinate system
  vec3fv Transformation::PointTransformFrom(const vec3fv &p) const 
  { 
    return packet_tm * p + packet_pos; 
  }
  // Transforms a vector to the local coordinate system 
  // (same as multiplication with the inverse transpose of the transformation)
  vec3fv Transformation::VectorTransformInTo(const vec3fv &v) const 
  { 
    return TransposeMult(packet_tm,  v); 
  }
  // Transforms a vector from the local coordinate system 
  // (same as multiplication with the inverse transpose of the transformation)
  vec3fv Transformation::VectorTransformFrom(const vec3fv &v) const 
  { 
    return TransposeMult(packet_itm, v);
  }

  //! Normal Calls
  void Transformation::Translate(const vec3f& p) { pos += p; }
  void Transformation::Rotate(const vec3f& axis, const float degree)
  { 
    mat4f m; glm::rotate(m, degree*(float)M_PI/180.0f, axis); Transform(mat3f(m));
  }
  void Transformation::Scale(const float sx, const float sy, const float sz)
  { 
    mat4f m(0.0f); glm::scale(m, vec3f(sx, sy, sz)); Transform(mat3f(m)); 
  }
  void Transformation::Transform(const mat3f& m) 
  { 
    tm = m * tm; pos = m * pos; itm = glm::inverse(tm);
  }
  void Transformation::InitTransform() 
  { 
    pos = vec3f(0.f); tm = mat3f(1.f); itm = mat3f(1.f); 
  }
  void Transformation:: FinalizeTransform() 
  { 
    packet_pos.x = pos.x; packet_pos.y = pos.y; packet_pos.z = pos.z;
    for (size_t i = 0; i < 9; ++i) {
      packet_tm[i]  =  tm[i];
      packet_itm[i] = itm[i];
    }
  }

  // Multiplies the given vector with the transpose of the given matrix
  vec3fv Transformation::TransposeMult(const mat3fv &m, const vec3fv &v)
  {
    vec3fv d;
    d.x = dot(glm::column(m, 0), v);
    d.y = dot(glm::column(m, 1), v);
    d.z = dot(glm::column(m, 2), v);
    return d;
  }
};

