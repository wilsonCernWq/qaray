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
/// \file       lights.h 
/// \author     Qi WU
///
/// \brief Modified Light clas for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "scene/scene_fwd.h"
#include "math/math.h"
#include "common/item.h"

//------------------------------------------------------------------------------

namespace qw {
  class Light : public ItemBase
  {
  public:
    virtual Color3fv Illuminate(const vec3fv &p, const vec3fv &N) const=0;
    virtual vec3fv   Direction (const vec3fv &p) const=0;
    virtual bool     IsAmbient () const { return false; }
    virtual void     SetViewportLight(int lightID) const {} // used for OpenGL display
  };

  class LightList : public ItemList<Light> {};
};

//------------------------------------------------------------------------------
