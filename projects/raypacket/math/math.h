//------------------------------------------------------------------------------
///
/// \file       math.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------

#include "scalar.h"
#include "vector.h"

//------------------------------------------------------------------------------

#ifndef USE_GLM
# error "Ray-packet project requires GLM"
#else

# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/matrix_access.hpp>
# include <glm/gtc/type_ptr.hpp>
# include <glm/gtx/norm.hpp>

#endif

//------------------------------------------------------------------------------ 
#define DEFINE_TYPE(TYPE, N, T)        \
  namespace qaray          \
  {              \
    typedef glm::tvec##N<TYPE> vec##N##T;    \
    typedef glm::tmat##N##x##N<TYPE> mat##N##T;    \
    struct bbox##N##T { vec##N##T upper, lower; };  \
    struct affine##N##T { mat##N##T l; vec##N##T p; };  \
  };

#define DEFINE_ALL_DEC(type, t)      \
  DEFINE_TYPE(type, 2, t)      \
  DEFINE_TYPE(type, 3, t)      \
  DEFINE_TYPE(type, 4, t)

DEFINE_ALL_DEC(unsigned char, c);
DEFINE_ALL_DEC(int, i);
DEFINE_ALL_DEC(float, f);
DEFINE_ALL_DEC(double, d);

namespace qaray
{
  typedef vec3f Color3f;
  typedef vec3c Color3c;
  typedef vec4f Color4f;
  typedef vec4c Color4c;
};

// TODO temporary
typedef qaray::vec2f Point2;
typedef qaray::vec3f Point3;
typedef qaray::vec4f Point4;
typedef qaray::mat3f Matrix3;
typedef qaray::mat4f Matrix4;
typedef qaray::Color3f Color;
typedef qaray::Color4f ColorA;
typedef qaray::Color3c Color24;
typedef unsigned char uchar;

inline Color ToColor (const Color24 &c)
{
  return Color(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
}
