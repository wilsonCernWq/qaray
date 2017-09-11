//------------------------------------------------------------------------------
///
/// \file       math.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#pragma once

#define PACKET_SIZE 16

#ifndef USE_LIBSIMDPP
# error "Ray-packet project requires libsimdpp"
#else
# define SIMDPP_ARCH_X86_AVX512F
# include <simdpp/simd.h>
#endif

#ifndef USE_GLM
# error "Ray-packet project requires GLM"
#else
# include <glm/glm.hpp>
#endif

//------------------------------------------------------------------------------
// -- here we have to extent the basic types
// -- with default initializations
namespace qw {
  class vfloat : public simdpp::float32<PACKET_SIZE> {
  private:
    using parent = simdpp::float32<PACKET_SIZE>;
  public:
    vfloat() = default;
    vfloat(const parent& x);
    vfloat(const float x);
    vfloat& operator=(const float x);
  };
};

//------------------------------------------------------------------------------
#define DEFINE_TYPE(TYPE, N, T)				\
  namespace qw						\
  {							\
    typedef glm::tvec##N<TYPE> vec##N##T;		\
    typedef glm::tmat##N##x##N<TYPE> mat##N##T;		\
    struct bbox##N##T { vec##N##T upper, lower; };	\
    struct affine##N##T { mat##N##T l; vec##N##T p; };	\
  };
#define DEFINE_ALL_TYPES(type, t)			\
  DEFINE_TYPE(type, 2, t)				\
  DEFINE_TYPE(type, 3, t)				\
  DEFINE_TYPE(type, 4, t)

// define all types
DEFINE_ALL_TYPES(qw::vfloat, fv);

namespace qw {
  vec3fv normalize(const vec3fv& v);
};
