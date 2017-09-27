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
# include <glm/gtc/type_ptr.hpp>
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

#define DEFINE_FUNC(TYPE, N, T)				\
  namespace qw {					\
  };

#define DEFINE_ALL_DEC(type, t)			\
  DEFINE_TYPE(type, 2, t)			\
  DEFINE_TYPE(type, 3, t)			\
  DEFINE_TYPE(type, 4, t)			\
  DEFINE_FUNC(type, 2, t)			\
  DEFINE_FUNC(type, 3, t)			\
  DEFINE_FUNC(type, 4, t)

DEFINE_ALL_DEC(unsigned char, c);
DEFINE_ALL_DEC(int,           i);
DEFINE_ALL_DEC(float,         f);
DEFINE_ALL_DEC(double,        d);

namespace qw {
  typedef vec3f  Color3f;
  typedef vec3c  Color3c;
  typedef vec4f  Color4f;
  typedef vec4c  Color4c;
};
using namespace qw;

#define make_vec2(x) glm::make_vec2(x)
#define make_vec3(x) glm::make_vec3(x)
#define make_vec4(x) glm::make_vec4(x)
#define normalize(x) glm::normalize(x)

typedef qw::vec2f Point2;
typedef qw::vec3f Point3;
typedef qw::vec4f Point4;
typedef qw::mat3f Matrix3;
typedef qw::Color3f Color;
typedef qw::Color4f ColorA;
typedef qw::Color3c Color24;
typedef unsigned char uchar;
