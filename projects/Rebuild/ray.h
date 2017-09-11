//------------------------------------------------------------------------------
///
/// \file       ray.h
/// \author     Qi WU
///
/// \brief This file defines the ray packet
///
//------------------------------------------------------------------------------

#pragma once

#include "math.h"

namespace qw {
  class RayPacket {
  public:
    vec3fv ori, dir;
  public:
    RayPacket();
    RayPacket(const vec3fv&, const vec3fv&);
    RayPacket(const RayPacket&);    
    void Normalize();    
  };
};
