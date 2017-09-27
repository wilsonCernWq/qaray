//------------------------------------------------------------------------------
///
/// \file       scalar.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#pragma once

#include "macro.h"

// #ifndef USE_LIBSIMDPP
// # error "Ray-packet project requires libsimdpp"
// #else
// # include <simdpp/simd.h>
// #endif

#include <type_traits>

//------------------------------------------------------------------------------
namespace qw 
{
  typedef unsigned char uchar;

  /* SIMD types */
  // For now we define vfloat as float  
  typedef float vfloat;
  typedef float vmask;
  
};

