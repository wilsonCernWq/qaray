//------------------------------------------------------------------------------
///
/// \file       ray.h
/// \author     Qi WU
///
/// \brief This file defines the ray packet
///
//------------------------------------------------------------------------------

#include "ray.h"
#include <glm/glm.hpp>

namespace qw {
  RayPacket::RayPacket() {};
  RayPacket::RayPacket(const vec3fv& p, const vec3fv& d) : ori(p), dir(d) {}
  RayPacket::RayPacket(const RayPacket &r) : ori(r.ori), dir(r.dir) {}
  void RayPacket::Normalize() { dir = normalize(dir); }   
};
