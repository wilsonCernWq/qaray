//-------------------------------------------------------------------------------
///
/// \file       lights.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    13.0
/// \date       November 20, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#ifndef _LIGHTS_H_INCLUDED_
#define _LIGHTS_H_INCLUDED_

#include "scene/scene.h"

//------------------------------------------------------------------------------

void DisableInverseSquareFalloff();

//------------------------------------------------------------------------------

class GenLight : public Light {
 public:
  static int shadow_spp_min;
  static int shadow_spp_max;
 protected:
  void SetViewportParam(int lightID, Color4f ambient,
                        Color4f intensity, Point4 pos) const;
  static float Shadow(Ray ray, float t_max = BIGFLOAT);
};

//------------------------------------------------------------------------------

class AmbientLight : public GenLight {
 public:
  AmbientLight() : intensity(0, 0, 0) {}

  Color3f Illuminate(const Point3 &p,
                     const Point3 &N) const override { return intensity; }

  Point3 Direction(const Point3 &p) const override { return Point3(0, 0, 0); }

  bool IsAmbient() const override { return true; }

  void SetViewportLight(int lightID) const override
  {
    SetViewportParam(lightID,
                     Color4f(intensity, 1.f),
                     Color4f(0.0f),
                     Point4(0, 0, 0, 1));
  }

  void SetIntensity(Color3f intens) { intensity = intens; }

 private:
  Color3f intensity;
};

//------------------------------------------------------------------------------

class DirectLight : public GenLight {
 public:
  DirectLight() : intensity(0, 0, 0), direction(0, 0, 1) {}

  Color3f Illuminate(const Point3 &p, const Point3 &N) const override
  {
    Ray ray(p, -direction);
    ray.Normalize();
    return Shadow(ray) * intensity;
  }

  Point3 Direction(const Point3 &p) const override { return direction; }

  void SetViewportLight(int lightID) const override
  {
    SetViewportParam(lightID,
                     Color4f(0.0f),
                     Color4f(intensity, 1.f),
                     Point4(-direction, 0.f));
  }

  void SetIntensity(Color3f intens) { intensity = intens; }

  void SetDirection(Point3 dir) { direction = glm::normalize(dir); }

 private:
  Color3f intensity;
  Point3 direction;
};

//------------------------------------------------------------------------------

class PointLight : public GenLight {
 public:
  PointLight() : intensity(0, 0, 0), position(0, 0, 0), size(0) {}

  Color3f Illuminate(const Point3 &p, const Point3 &N) const override;

  Point3 Direction(const Point3 &p) const override
  {
    return glm::normalize(p - position);
  }

  void SetViewportLight(int lightID) const override;

  void SetIntensity(Color3f intens) { intensity = intens; }

  void SetPosition(Point3 pos) { position = pos; }

  void SetSize(float s) { size = s; }

  // Photon Extensions
  bool IsPhotonSource() const override { return true; }
  Color3f GetPhotonIntensity() const override { return intensity; }
  DiffRay RandomPhoton() const override;

 private:
  Color3f intensity;
  Point3 position;
  float size;
};

//------------------------------------------------------------------------------

#endif
