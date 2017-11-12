//------------------------------------------------------------------------------
///
/// \file       materials.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 11, 2015
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
    diffuse(0.5f,0.5f,0.5f),
    specular(0.7f,0.7f,0.7f),
    glossiness(20.0f),
    emission(0,0,0),
    reflection(0,0,0),
    refraction(0,0,0),
    absorption(0,0,0),
    ior(1),
    reflectionGlossiness(0),
    refractionGlossiness(0) {}

  virtual Color Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
		      const LightList &lights, int bounceCount) const;
  
  void SetDiffuse(Color dif){ diffuse.SetColor(dif); }
  void SetSpecular(Color spec){ specular.SetColor(spec); }
  void SetGlossiness  (float gloss){ glossiness = gloss; }
  void SetEmission(Color e){ emission.SetColor(e); }
  
  void SetReflection(Color reflect){ reflection.SetColor(reflect); }
  void SetRefraction(Color refract){ refraction.SetColor(refract); }
  void SetAbsorption(Color absorp ){ absorption = absorp; }
  void SetRefractionIndex(float _ior) { ior = _ior; }

  void SetDiffuseTexture (TextureMap *map){ diffuse.SetTexture(map); }
  void SetSpecularTexture (TextureMap *map){ specular.SetTexture(map); }
  void SetEmissionTexture (TextureMap *map){ emission.SetTexture(map); }
  void SetReflectionTexture(TextureMap *map){ reflection.SetTexture(map); }
  void SetRefractionTexture(TextureMap *map){ refraction.SetTexture(map); }
  void SetReflectionGlossiness(float gloss){ reflectionGlossiness=gloss; }
  void SetRefractionGlossiness(float gloss){ refractionGlossiness=gloss; }
  
  virtual void SetViewportMaterial(int subMtlID=0) const; // used for OpenGL display

private:
  TexturedColor diffuse, specular, reflection, refraction, emission;
  float glossiness;
  Color absorption;
  float ior;	// index of refraction
  float reflectionGlossiness, refractionGlossiness;
};

//------------------------------------------------------------------------------

class MtlPhong : public Material
{
public:
  MtlPhong() :
  diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f),
  reflection(0,0,0), refraction(0,0,0),
  glossiness(20.0f), absorption(0,0,0), ior(1) {}
  virtual Color Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
		      const LightList &lights, int bounceCount) const;
  
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

class MultiMtl : public Material
{
public:
  virtual ~MultiMtl() { for ( unsigned int i=0; i<mtls.size(); i++ ) delete mtls[i]; }
  virtual Color Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
		      const LightList &lights, int bounceCount) const
  {
    return hInfo.c.mtlID < (int)mtls.size() ?
      mtls[hInfo.c.mtlID]->Shade(ray,hInfo,lights,bounceCount) :
      Color(1,1,1);
  }
  virtual void SetViewportMaterial(int subMtlID=0) const
  {
    if ( subMtlID<(int)mtls.size() ) mtls[subMtlID]->SetViewportMaterial();
  }
  void AppendMaterial(Material *m) { mtls.push_back(m); }
private:
  std::vector<Material*> mtls;
};

//------------------------------------------------------------------------------

#endif
