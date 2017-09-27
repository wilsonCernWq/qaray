//-------------------------------------------------------------------------------
///
/// \file       lights.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    3.0
/// \date       September 4, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#ifndef _LIGHTS_H_INCLUDED_
#define _LIGHTS_H_INCLUDED_

#include "scene.h"

//------------------------------------------------------------------------------

class GenLight : public Light
{
protected:
  void SetViewportParam(int lightID, ColorA ambient, ColorA intensity, Point4 pos) const;
  static float Shadow(Ray ray, float t_max=BIGFLOAT);
};

//------------------------------------------------------------------------------

class AmbientLight : public GenLight
{
public:
  AmbientLight() : intensity(0,0,0) {}
  virtual Color Illuminate(const Point3 &p, const Point3 &N) const { return intensity; }
  virtual Point3 Direction(const Point3 &p) const { return Point3(0,0,0); }
  virtual bool IsAmbient() const { return true; }
  virtual void SetViewportLight(int lightID) const {
    SetViewportParam(lightID,ColorA(intensity),ColorA(0.0f),Point4(0,0,0,1));
  }  
  void SetIntensity(Color intens) { intensity=intens; }
private:
	Color intensity;
};

//------------------------------------------------------------------------------

class DirectLight : public GenLight
{
public:
  DirectLight() : intensity(0,0,0), direction(0,0,1) {}
  virtual Color Illuminate(const Point3 &p, const Point3 &N) const {
    Ray ray(p,-direction); ray.Normalize();
    return Shadow(ray) * intensity;
  }
  virtual Point3 Direction(const Point3 &p) const { return direction; }
  virtual void SetViewportLight(int lightID) const {
    SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Point4(-direction,0.0f));
  }  
  void SetIntensity(Color intens) { intensity=intens; }
  void SetDirection(Point3 dir) { direction=dir.GetNormalized(); }
private:
  Color intensity;
  Point3 direction;
};

//------------------------------------------------------------------------------

class PointLight : public GenLight
{
public:
  PointLight() : intensity(0,0,0), position(0,0,0) {}
  virtual Color Illuminate(const Point3 &p, const Point3 &N) const {
    Ray ray(p,position-p); ray.Normalize();
    return Shadow(ray,(position-p).Length()) * intensity;
  }
  virtual Point3 Direction(const Point3 &p) const { return (p-position).GetNormalized(); }
  virtual void SetViewportLight(int lightID) const {
    SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Point4(position,1.0f));
  }
  void SetIntensity(Color intens) { intensity=intens; }
  void SetPosition(Point3 pos) { position=pos; }
private:
  Color intensity;
  Point3 position;
};

//------------------------------------------------------------------------------

#endif
