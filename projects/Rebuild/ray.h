//------------------------------------------------------------------------------
///
/// \file       ray.h
/// \author     Qi WU
///
/// \brief This file defines the ray packet
///
//------------------------------------------------------------------------------

#pragma once

#include "common.h"
#include <simdpp/simd.h>

class RayPacket {
public:
  simdpp::float32<PACK_SIZE> x, y, z;
public:
  RayPacket();
};
