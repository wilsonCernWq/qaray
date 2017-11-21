//-------------------------------------------------------------------------------
///
/// \file       lights.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    10.0
/// \date       October 30, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#ifndef _LIGHTS_H_INCLUDED_
#define _LIGHTS_H_INCLUDED_

#include "scene.h"

//------------------------------------------------------------------------------

void DisableInverseSquareFalloff();

//------------------------------------------------------------------------------

class GenLight : public Light
{
public:
  static int shadow_spp_min;
  static int shadow_spp_max;
protected:
  void SetViewportParam (int lightID, ColorA ambient, ColorA intensity, Point4 pos) const;
  static float Shadow (Ray ray, float t_max = BIGFLOAT);
};

//------------------------------------------------------------------------------

class AmbientLight : public GenLight
{
public:
  AmbientLight () : intensity(0, 0, 0) {}

  virtual Color Illuminate (const Point3 &p, const Point3 &N) const { return intensity; }

  virtual Point3 Direction (const Point3 &p) const { return Point3(0, 0, 0); }

  virtual bool IsAmbient () const { return true; }

  virtual void SetViewportLight (int lightID) const
  {
    SetViewportParam(lightID, ColorA(intensity, 1.f), ColorA(0.0f), Point4(0, 0, 0, 1));
  }

  void SetIntensity (Color intens) { intensity = intens; }

private:
  Color intensity;
};

//------------------------------------------------------------------------------

class DirectLight : public GenLight
{
public:
  DirectLight () : intensity(0, 0, 0), direction(0, 0, 1) {}

  virtual Color Illuminate (const Point3 &p, const Point3 &N) const
  {
    Ray ray(p, -direction);
    ray.Normalize();
    return Shadow(ray) * intensity;
  }

  virtual Point3 Direction (const Point3 &p) const { return direction; }

  virtual void SetViewportLight (int lightID) const
  {
    SetViewportParam(lightID, ColorA(0.0f), ColorA(intensity, 1.f), Point4(-direction, 0.f));
  }

  void SetIntensity (Color intens) { intensity = intens; }

  void SetDirection (Point3 dir) { direction = glm::normalize(dir); }

private:
  Color intensity;
  Point3 direction;
};

//------------------------------------------------------------------------------

class PointLight : public GenLight
{
public:
  PointLight () : intensity(0, 0, 0), position(0, 0, 0), size(0) {}

  virtual Color Illuminate (const Point3 &p, const Point3 &N) const;

  virtual Point3 Direction (const Point3 &p) const
  {
    return glm::normalize(p - position);
  }

  virtual void SetViewportLight (int lightID) const
  {
    SetViewportParam(lightID, ColorA(0.0f), ColorA(intensity, 1.f), Point4(position, 1.f));
  }

  void SetIntensity (Color intens) { intensity = intens; }

  void SetPosition (Point3 pos) { position = pos; }

  void SetSize (float s) { size = s; }

private:
  Color intensity;
  Point3 position;
  float size;
};

//------------------------------------------------------------------------------

#endif
