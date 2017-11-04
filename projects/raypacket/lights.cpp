//------------------------------------------------------------------------------
///
/// \file       objects.cpp 
/// \author     Qi WU
/// \version    1.0
/// \date       August, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "lights.h"

extern Node rootNode;

//------------------------------------------------------------------------------

float GenLight::Shadow(Ray ray, float t_max)
{
  HitInfo hInfo; hInfo.z = t_max;
  if (TraceNodeShadow(rootNode, ray, hInfo)) {
    return 0.0f;
  } else {
    return 1.0f;
  }
}

//------------------------------------------------------------------------------

Color PointLight::Illuminate(const Point3 &p, const Point3 &N) const
{
  Point3 dp;
  do {
    dp.x = rng->Get() * size;
    dp.y = rng->Get() * size;
    dp.z = rng->Get() * size;	
  } while (glm::length(dp) > size);  
  Ray ray(p,position+dp-p); ray.Normalize();
  return Shadow(ray,glm::length(position+dp-p)) * intensity;
}

//------------------------------------------------------------------------------
