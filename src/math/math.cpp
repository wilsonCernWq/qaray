//------------------------------------------------------------------------------
///
/// \file       algebra.cpp
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#include "math/math.h"

namespace qaray {
const float PI = static_cast<float>(M_PI);
const float RCP_PI = 1.f / PI;
const float RCP_2PI = 1.f / (2.f * PI);
};

//! Explicit instantiation
#define DEFINE_TYPE_DEF(TYPE, N, T)    \
  namespace glm          \
  {            \
    template struct tvec##N<TYPE>;    \
    template struct tmat##N##x##N<TYPE>;  \
  };

#define DEFINE_ALL_DEF(type, t)    \
  DEFINE_TYPE_DEF(type, 2, t);    \
  DEFINE_TYPE_DEF(type, 3, t);    \
  DEFINE_TYPE_DEF(type, 4, t);

DEFINE_ALL_DEF(unsigned char, c);
DEFINE_ALL_DEF(int, i);
DEFINE_ALL_DEF(float, f);
DEFINE_ALL_DEF(double, d);

namespace qaray {
Point3 TransformToLocalFrame(const Point3 &N, const Point3 &sample)
{
  const Point3 Z = N;
  const Point3 Y = (ABS(Z.x) > ABS(Z.y)) ?
                   normalize(Point3(Z.z, 0, -Z.x)) :
                   normalize(Point3(0, -Z.z, Z.y));
  const Point3 X = normalize(cross(Y, Z));
  const Point3 unit = normalize(sample);
  return unit.x * X + unit.y * Y + unit.z * Z;
}
}