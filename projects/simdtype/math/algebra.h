//------------------------------------------------------------------------------
///
/// \file       algebra.h
/// \author     Qi WU
///
/// \brief Math library used in this project
/// In this file, the linear algebra types (vector & matrix) are defined
///
//------------------------------------------------------------------------------

#pragma once

#include "scalar.h"

#ifndef USE_GLM
# error "Ray-packet project requires GLM"
#else

# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/matrix_access.hpp>

#endif

//------------------------------------------------------------------------------
#define DEFINE_TYPE(TYPE, N, T)        \
  namespace qw            \
  {              \
    typedef glm::tvec##N<TYPE> vec##N##T;    \
    typedef glm::tmat##N##x##N<TYPE> mat##N##T;    \
    struct bbox##N##T { vec##N##T upper, lower; };  \
    struct affine##N##T { mat##N##T l; vec##N##T p; };  \
  };

#define DEFINE_FUNC(TYPE, N, T)        \
  namespace qw {          \
    vec##N##T normalize(const vec##N##T&);    \
    TYPE dot(const vec##N##T&, const vec##N##T&);  \
  };

#define DEFINE_ALL_DEC(type, t)      \
  DEFINE_TYPE(type, 2, t)      \
  DEFINE_TYPE(type, 3, t)      \
  DEFINE_TYPE(type, 4, t)      \
  DEFINE_FUNC(type, 2, t)      \
  DEFINE_FUNC(type, 3, t)      \
  DEFINE_FUNC(type, 4, t)

DEFINE_ALL_DEC(qw::vfloat, fv);
DEFINE_ALL_DEC(uchar, c);
DEFINE_ALL_DEC(int, i);
DEFINE_ALL_DEC(float, f);
DEFINE_ALL_DEC(double, d);

namespace qw
{
  typedef vec3fv Color3fv;
  typedef vec3f Color3f;
  typedef vec3c Color3c;
};
