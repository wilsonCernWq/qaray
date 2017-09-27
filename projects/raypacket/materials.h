//------------------------------------------------------------------------------
///
/// \file       materials.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    4.0
/// \date       September 17, 2015
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#ifndef _MATERIALS_H_INCLUDED_
#define _MATERIALS_H_INCLUDED_

#include "scene.h"

//------------------------------------------------------------------------------

class MtlBlinn : public Material
{
public:
  MtlBlinn() :
  diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f),
  reflection(0,0,0), refraction(0,0,0),
  glossiness(20.0f), absorption(0,0,0), ior(1) {}
  virtual Color Shade(const Ray &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const;

  void SetDiffuse(Color dif) { diffuse = dif; }
  void SetSpecular(Color spec) { specular = spec; }
  void SetGlossiness(float gloss) { glossiness = gloss; }

  void SetReflection(Color reflect) { reflection = reflect; }
  void SetRefraction(Color refract) { refraction = refract; }
  void SetAbsorption(Color absorp ) { absorption = absorp; }
  void SetRefractionIndex(float _ior) { ior = _ior; }

  virtual void SetViewportMaterial(int subMtlID=0) const;	// used for OpenGL display

private:
  Color diffuse, specular, reflection, refraction;
  float glossiness;
  Color absorption;
  float ior;	// index of refraction
};

//------------------------------------------------------------------------------

class MtlPhong : public Material
{
public:
  MtlPhong() :
  diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f),
  reflection(0,0,0), refraction(0,0,0),
  glossiness(20.0f), absorption(0,0,0), ior(1) {}
  virtual Color Shade(const Ray &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const;
  
  void SetDiffuse(Color dif) { diffuse = dif; }
  void SetSpecular(Color spec) { specular = spec; }
  void SetGlossiness(float gloss) { glossiness = gloss; }

  void SetReflection(Color reflect) { reflection = reflect; }
  void SetRefraction(Color refract) { refraction = refract; }
  void SetAbsorption(Color absorp ) { absorption = absorp; }
  void SetRefractionIndex(float _ior) { ior = _ior; }
  
  virtual void SetViewportMaterial(int subMtlID=0) const;// used for OpenGL display
  
private:
  Color diffuse, specular, reflection, refraction;
  float glossiness;
  Color absorption;
  float ior;	// index of refraction
};

//------------------------------------------------------------------------------

#endif
