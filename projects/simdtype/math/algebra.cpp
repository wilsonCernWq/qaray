//------------------------------------------------------------------------------
///
/// \file       algebra.cpp
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#include "algebra.h"

//! Explicit instantiation
#define DEFINE_TYPE_DEF(TYPE, N, T)  \
  namespace glm        \
  {          \
    template struct tvec##N<TYPE>;  \
    template struct tmat##N##x##N<TYPE>;\
  };

//! Function definitions
#define DEFINE_FUNC_DEF(TYPE, N, T)      \
  namespace qw {          \
    vec##N##T normalize(const vec##N##T& v) {    \
      TYPE s(0);          \
      for (int i = 0; i < N; ++i) {      \
  s = s + v[i];          \
      }              \
      return v / s;          \
    }              \
    TYPE dot(const vec##N##T& a, const vec##N##T& b) {  \
      TYPE s(0);          \
      for (int i = 0; i < N; ++i) {      \
  s = s + a[i] * b[i];        \
      }              \
      return s;            \
    }              \
  };

#define DEFINE_ALL_DEF(type, t)    \
  DEFINE_TYPE_DEF(type, 2, t);    \
  DEFINE_TYPE_DEF(type, 3, t);    \
  DEFINE_TYPE_DEF(type, 4, t);    \
  DEFINE_FUNC_DEF(type, 2, t);    \
  DEFINE_FUNC_DEF(type, 3, t);    \
  DEFINE_FUNC_DEF(type, 4, t);

DEFINE_ALL_DEF(qw::vfloat, fv);
DEFINE_ALL_DEF(int, i);
DEFINE_ALL_DEF(float, f);
DEFINE_ALL_DEF(double, d);

