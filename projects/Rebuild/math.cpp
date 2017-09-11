//------------------------------------------------------------------------------
///
/// \file       math.cpp
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#include "math.h"

namespace qw {

  vfloat::vfloat(const vfloat::parent& x) { parent::operator=(x); };
  vfloat::vfloat(const float x) { parent::operator=(simdpp::splat(x)); }
  vfloat& vfloat::operator=(const float x) { 
    parent::operator=(simdpp::splat(x)); 
    return *this; 
  }

  vec3fv normalize(const vec3fv& v) { 
    vfloat s(v.x + v.y + v.z);
    return v / s;
  }

};
