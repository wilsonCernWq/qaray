//------------------------------------------------------------------------------
///
/// \file       vec.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#pragma once

#include "scalar.h"

#ifndef USE_GLM
# error "Ray-packet project requires GLM"
#else
# include <glm/glm.hpp>
#endif

//------------------------------------------------------------------------------
#define DEFINE_TYPE(TYPE, N, T)				\
  namespace qw						\
  {							\
    typedef glm::tvec##N<TYPE> vec##N##T;		\
    typedef glm::tmat##N##x##N<TYPE> mat##N##T;		\
    struct bbox##N##T { vec##N##T upper, lower; };	\
    struct affine##N##T { mat##N##T l; vec##N##T p; };	\
  };

#define DEFINE_FUNC(TYPE, N, T)			\
  namespace qw {				\
    void normalize(vec##N##T&);			\
  };

#define DEFINE_ALL_DEC(type, t)			\
  DEFINE_TYPE(type, 2, t)			\
  DEFINE_TYPE(type, 3, t)			\
  DEFINE_TYPE(type, 4, t)			\
  DEFINE_FUNC(type, 2, t)			\
  DEFINE_FUNC(type, 3, t)			\
  DEFINE_FUNC(type, 4, t)

DEFINE_ALL_DEC(qw::vfloat, fv);
