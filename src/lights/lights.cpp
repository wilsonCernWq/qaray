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

//------------------------------------------------------------------------------

int GenLight::shadow_spp_min = 16;
int GenLight::shadow_spp_max = 64;
static bool useInverseSquareFalloff = true;
static float InverseSquareDistance = 1.f;

void DisableInverseSquareFalloff() { useInverseSquareFalloff = false; }

float InverseSquareFalloff(const Point3 &v)
{
  if (useInverseSquareFalloff) {
    return MIN(InverseSquareDistance, InverseSquareDistance / length2(v));
  } else {
    return 1.f;
  }
}

float SphereFalloff(const float r, const float D)
{
  return (D - r) / D;
}


//------------------------------------------------------------------------------
float GenLight::Shadow(Ray ray, float t_max)
{
  HitInfo hInfo;
  hInfo.z = t_max;
  if (scene.TraceNodeShadow(scene.rootNode, ray, hInfo)) {
    return 0.0f;
  } else {
    return 1.0f;
  }
}
//------------------------------------------------------------------------------
Color3f PointLight::Illuminate(const Point3 &p, const Point3 &N) const
{
  if (size > 0.01f) {
    int spp = GenLight::shadow_spp_min, s = 0;
    float inshadow = 0.0f;
    while (s < spp) {
      const Point3 dir = position + rng->local().UniformBall(size) - p;
      Ray ray(p, dir);
      ray.Normalize();
      inshadow += (Shadow(ray, length(dir)) - inshadow) *
          InverseSquareFalloff(dir) /
          (float) (s + 1);
      s++;
      if (inshadow > 0.f && inshadow < 1.f) { spp = GenLight::shadow_spp_max; }
    };
    return inshadow * intensity;
  } else {
    const Point3 dir = position - p;
    Ray ray(p, dir);
    ray.Normalize();
    return Shadow(ray, length(dir)) *
        intensity *
        InverseSquareFalloff(dir);
  }
}
//------------------------------------------------------------------------------
DiffRay PointLight::RandomPhoton() const
{
  Point3 dir = rng->local().UniformSphere();
  return DiffRay(position, dir);
}
//------------------------------------------------------------------------------

Color3f SpotLight::Illuminate(const Point3 &p, const Point3 &N) const
{
  Color3f I;
  // same as point light
  if (size > 0.01f) {
    int spp = GenLight::shadow_spp_min, s = 0;
    float inshadow = 0.0f;
    while (s < spp) {
      const Point3 dir = position + rng->local().UniformBall(size) - p;
      Ray ray(p, dir);
      ray.Normalize();
      inshadow +=
          (Shadow(ray, length(dir)) - inshadow) *
              InverseSquareFalloff(dir) / (float) (s + 1);
      s++;
      if (inshadow > 0.f && inshadow < 1.f) { spp = GenLight::shadow_spp_max; }
    };
    I = inshadow * intensity;
  } else {
    const Point3 dir = position - p;
    Ray ray(p, dir);
    ray.Normalize();
    I = Shadow(ray, length(dir)) * intensity * InverseSquareFalloff(dir);
  }
  // calculate spot light attenuation
  return I * GetAttenuation(Direction(p));
}
DiffRay SpotLight::RandomPhoton() const
{
  Point3 dir = rng->local().UniformHemisphere();
  return DiffRay(position, TransformToLocalFrame(direction, dir));
}
void SpotLight::SetRotation(float degree, Point3 axis)
{
  direction = normalize
      (glm::rotate(mat4f(1.f), glm::radians(degree), axis) * vec4f(0,0,-1,0));
}
void SpotLight::SetAngle(float s) {
  s = MIN(MAX(s/2.f, 1.f), 89.f) / 180.f * PI;
  outer = tan(s);
}
void SpotLight::SetBlend(float s) {
  blend = MAX(MIN(s, 1.f), 0.f);
  inner = SQRT(outer * outer * (1.f-blend));
}
float SpotLight::GetAttenuation(const Point3& dir) const
{
  const float cosTheta = dot(dir, direction);
  if (cosTheta < 0)
  {
    return 0;
  }
  else {
    const float r = SQRT(1.f - cosTheta * cosTheta) / cosTheta;
    if (r > outer) {
      return 0;
    } else {
      return r < inner ? 1.f : (outer - r) / (outer - inner);
    }

  }
}