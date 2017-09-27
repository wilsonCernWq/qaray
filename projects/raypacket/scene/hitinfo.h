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
/// \file       hitinfo.h 
/// \author     Qi WU
///
/// \brief Modified for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "macro.h"
#include "scene/scene_fwd.h"
#include "math/math.h"

namespace qw {
  struct HitInfo
  {
    union NodeInfoPack {
      float out;
      struct { const Node* node; int front; } in;
    };
    vfloat z;	   // the distance from the ray center to the hit point
    vec3fv p;	   // position of the hit point
    vec3fv N;	   // surface normal at the hit point
    vfloat nodeInfo; // [const Node *node, front] ==> pack of (void*) & (int)
    // [node ] the object node that was hit
    // [front] true if the ray hits the front side, false if the ray hits the back side
    HitInfo() { Init(); }
    void Init() { 
      z=BIGFLOAT; 
      nodeInfo = Encode(NULL,true);
    }
    float Encode(const Node* node, bool front) {
      NodeInfoPack u;
      u.in.node  = node;
      u.in.front = static_cast<int>(front);
      return u.out;
    }
  };
};
