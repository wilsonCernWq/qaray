//------------------------------------------------------------------------------
///
/// \file       vec.cpp
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#include "vec.h"

//! Explicit instantiation
#define DEFINE_TYPE_DEF(TYPE, N, T)	\
  namespace glm				\
  {					\
    template struct tvec##N<TYPE>;	\
    template struct tmat##N##x##N<TYPE>;\
  };

//! Function definitions
#define DEFINE_FUNC_DEF(TYPE, N, T)	\
  namespace qw {			\
    void normalize(vec##N##T& v) {	\
      TYPE s(0);			\
      for (size_t i = 0; i < N; ++i) {	\
	s = s + v[i];			\
      }					\
      v = v / s;			\
    }					\
  };

#define DEFINE_ALL_DEF(type, t)		\
  DEFINE_TYPE_DEF(type, 2, t);		\
  DEFINE_TYPE_DEF(type, 3, t);		\
  DEFINE_TYPE_DEF(type, 4, t);		\
  DEFINE_FUNC_DEF(type, 2, t);		\
  DEFINE_FUNC_DEF(type, 3, t);		\
  DEFINE_FUNC_DEF(type, 4, t);

DEFINE_ALL_DEF(qw::vfloat, fv);

