#pragma once

#include "scene.h"

//------------------------------------------------------------------------------

class MtlBlinn_Basic : public Material {
 public:
  MtlBlinn_Basic();

  void SetDiffuse(Color dif) { diffuse.SetColor(dif); }

  void SetSpecular(Color spec) { specular.SetColor(spec); }

  void SetGlossiness(float gloss) { glossiness = gloss; }

  void SetEmission(Color e) { emission.SetColor(e); }

  void SetReflection(Color reflect) { reflection.SetColor(reflect); }

  void SetRefraction(Color refract) { refraction.SetColor(refract); }

  void SetAbsorption(Color absorp) { absorption = absorp; }

  void SetRefractionIndex(float _ior) { ior = _ior; }

  void SetDiffuseTexture(TextureMap *map) { diffuse.SetTexture(map); }

  void SetSpecularTexture(TextureMap *map) { specular.SetTexture(map); }

  void SetEmissionTexture(TextureMap *map) { emission.SetTexture(map); }

  void SetReflectionTexture(TextureMap *map) { reflection.SetTexture(map); }

  void SetRefractionTexture(TextureMap *map) { refraction.SetTexture(map); }

  void SetReflectionGlossiness(float gloss) { reflectionGlossiness = gloss; }

  void SetRefractionGlossiness(float gloss) { refractionGlossiness = gloss; }

  virtual Color Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                      const LightList &lights, int bounceCount) const;

  virtual void SetViewportMaterial(int subMtlID = 0) const; // used for OpenGL display
 private:
  TexturedColor diffuse, specular, reflection, refraction, emission;
  float glossiness;
  Color absorption;
  float ior;  // index of refraction
  float reflectionGlossiness, refractionGlossiness;
};

//------------------------------------------------------------------------------
