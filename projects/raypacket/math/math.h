//------------------------------------------------------------------------------
///
/// \file       math.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#ifndef _QARAY_MATH_MATH_H_
#define _QARAY_MATH_MATH_H_
#pragma once

//------------------------------------------------------------------------------

#ifndef _USE_MATH_DEFINES
# define _USE_MATH_DEFINES
#endif

#include <cstdint>
#include <cmath>
#include <limits>
#include <algorithm>

typedef bool    qaBOOL;
typedef uint8_t qaUCHAR;
typedef int     qaINT;
typedef float   qaFLOAT;
typedef double  qaDOUBLE;

namespace qaray {

template<typename T>
T min(const T &x, const T &y) { return (x < y ? x : y); }

template<typename T>
T max(const T &x, const T &y) { return (x > y ? x : y); }

template<typename T>
T clamp(const T &X, const T &L, const T &U) { return min(U, max(L, X)); }

template<typename T>
T abs(const T &x) { return (x > 0 ? x : -x); }

template<typename T>
T pow(const T &x, const T &y) { return std::pow(x, y); }

template<typename T>
T sqrt(const T &x) { return std::sqrt(x); }

template<typename T>
T ceil(const T &x) { return std::ceil(x); }

template<typename T>
T floor(const T &x) { return std::floor(x); }

template<typename T>
T sin(const T &x) { return std::sin(x); }

template<typename T>
T cos(const T &x) { std::cos(x); }

const qaFLOAT BIGFLOAT = std::numeric_limits<qaFLOAT>::max();
const qaFLOAT PI = static_cast<float>(M_PI);
}

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

#define DEFINE_TYPE(TYPE, N, T)                         \
  namespace qaray {                                     \
    typedef glm::tvec##N<TYPE> vec##N##T;               \
    typedef glm::tmat##N##x##N<TYPE> mat##N##T;         \
    struct bbox##N##T { vec##N##T upper, lower; };      \
    struct affine##N##T { mat##N##T l; vec##N##T p; };  \
  };

#define DEFINE_ALL_DEC(type, t) \
  DEFINE_TYPE(type, 2, t)       \
  DEFINE_TYPE(type, 3, t)       \
  DEFINE_TYPE(type, 4, t)

DEFINE_ALL_DEC(qaUCHAR, c);
DEFINE_ALL_DEC(qaINT,   i);
DEFINE_ALL_DEC(qaFLOAT, f);
DEFINE_ALL_DEC(qaDOUBLE,d);

namespace qaray {

// Porting the Data Type From Previous Program
typedef qaray::vec2f Point2;
typedef qaray::vec3f Point3;
typedef qaray::vec4f Point4;
typedef qaray::mat3f Matrix3;
typedef qaray::mat4f Matrix4;

typedef vec3f Color3f;
typedef vec4f Color4f;
typedef vec3c Color3c;
typedef vec4c Color4c;

inline Color3f ToColor3f(const Color3c& c) {
  return Color3f(c.r/255.f, c.g/255.f, c.b/255.f);
}

inline Color3f ToColor3c(const Color3f& c) {
  return Color3c(static_cast<qaUCHAR >(c.r * 255),
                 static_cast<qaUCHAR >(c.g * 255),
                 static_cast<qaUCHAR >(c.b * 255));
}

template<typename T>
qaFLOAT length(const T &x) { glm::length(x); }
};

using namespace qaray;

#endif//_QARAY_MATH_MATH_H_