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
/// \file       materials.h 
/// \author     Qi WU
///
/// \brief Modified Material class for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "scene/scene_fwd.h"
#include "scene/hitinfo.h"
#include "common/ray.h"

//------------------------------------------------------------------------------

namespace qw {
  class Material : public ItemBase
  {
  public:
    // The main method that handles the shading by calling all the lights in the list.
    // ray: incoming ray,
    // hInfo: hit information for the point that is being shaded, lights: the light list,
    virtual Color3fv Shade(const RayPacket &ray, 
			   const HitInfo &hInfo, 
			   const LightList &lights) const = 0;
    virtual void SetViewportMaterial(const int subMtlID = 0) const {} // used for OpenGL display
  };

  class MaterialList : public ItemList<Material>
  {
  public:
    Material* Find(const char *name) { 
      int n = size(); 
      for (int i=0; i<n; i++)
      {	
	if (at(i) && strcmp(name,at(i)->GetName())==0) { return at(i); }
      } 
      return NULL; 
    }
  };
}

//------------------------------------------------------------------------------
