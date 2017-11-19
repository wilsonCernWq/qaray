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

int GenLight::shadow_spp_min = 16;
int GenLight::shadow_spp_max = 64;

float InverseSquareFalloff (const Point3 &v)
{
  return MIN(160.f, 160.f / glm::length2(v));
}

float GenLight::Shadow (Ray ray, float t_max)
{
  HitInfo hInfo;
  hInfo.z = t_max;
  if (TraceNodeShadow(rootNode, ray, hInfo))
  {
    return 0.0f;
  } else
  {
    return 1.0f;
  }
}

//------------------------------------------------------------------------------

Color PointLight::Illuminate (const Point3 &p, const Point3 &N) const
{
  if (size > 0.01f)
  {
    int spp = GenLight::shadow_spp_min, s = 0;
    float inshadow = 0.0f;
    while (s < spp)
    {
      const float r1 = rng->Get();
      const float r2 = rng->Get();
      const float r3 = rng->Get();
      const Point3 dir = position + GetCirclePoint(r1, r2, r3, size) - p;
      Ray ray(p, dir);
      ray.Normalize();
      inshadow += (Shadow(ray, glm::length(dir)) - inshadow) *
                  InverseSquareFalloff(dir) /
                  (float) (s + 1);
      s++;
      if (inshadow > 0.f && inshadow < 1.f) { spp = GenLight::shadow_spp_max; }
    };
    return inshadow * intensity;
  } else
  {
    const Point3 dir = position - p;
    Ray ray(p, dir);
    ray.Normalize();
    return Shadow(ray, glm::length(dir)) *
           intensity *
           InverseSquareFalloff(dir);
  }
}

//------------------------------------------------------------------------------
