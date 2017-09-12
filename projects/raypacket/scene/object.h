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
/// \file       scene.h 
/// \author     Qi WU
///
/// \brief Modified for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "scene/scene_fwd.h"
#include "scene/hitinfo.h"
#include "common/ray.h"
#include "common/item.h"

//------------------------------------------------------------------------------
// Base class for all object types
namespace qw {
  class Object
  {
  public:
    virtual bool IntersectRay(const RayPacket &ray, HitInfo &hInfo, int hitSide=HIT_FRONT) const = 0;
    virtual void ViewportDisplay(const Material *mtl) const {} // used for OpenGL display
  };

  typedef ItemFileList<Object> ObjFileList;
};
//------------------------------------------------------------------------------
