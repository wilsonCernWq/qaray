//------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.1
/// \date       August 29, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///
/// \file       setup.h 
/// \author     Qi WU
///
/// \brief Setup constants
///
//------------------------------------------------------------------------------

#pragma once

#define SIMDPP_ARCH_X86_AVX512F
#define PACKET_SIZE 16

#define HIT_NONE           1<<0
#define HIT_FRONT          1<<1
#define HIT_BACK           1<<2
#define HIT_FRONT_AND_BACK (HIT_FRONT|HIT_BACK)

#define BIGFLOAT 1.0e30f
