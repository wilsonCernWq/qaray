///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/4/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.              //
///                                                                          //
/// Redistribution and use in source and binary forms, with or without       //
/// modification, are permitted provided that the following conditions are   //
/// met:                                                                     //
///  - Redistributions of source code must retain the above copyright        //
///    notice, this list of conditions and the following disclaimer.         //
///  - Redistributions in binary form must reproduce the above copyright     //
///    notice, this list of conditions and the following disclaimer in the   //
///    documentation and/or other materials provided with the distribution.  //
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS  //
/// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    //
/// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          //
/// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       //
/// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         //
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    //
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    //
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      //
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    //
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     //
///                                                                          //
///--------------------------------------------------------------------------//

#ifndef QARAY_MTLBLINN_PHOTON_MAP_H
#define QARAY_MTLBLINN_PHOTON_MAP_H
#pragma once

#include "scene/scene.h"

namespace qaray {
class MtlBlinn_PhotonMap : public Material {
public:
  MtlBlinn_PhotonMap();

  void SetDiffuse(Color3f dif) { diffuse.SetColor(dif); }

  void SetSpecular(Color3f spec) { specular.SetColor(spec); }

  void SetGlossiness(float gloss) { specularGlossiness = gloss; }

  void SetEmission(Color3f emit) { emission.SetColor(emit); }

  void SetReflection(Color3f reflect) { reflection.SetColor(reflect); }

  void SetRefraction(Color3f refract) { refraction.SetColor(refract); }

  void SetAbsorption(Color3f absorb) { absorption = absorb; }

  void SetRefractionIndex(float idx) { ior = idx; }

  void SetDiffuseTexture(TextureMap *map) { diffuse.SetTexture(map); }

  void SetSpecularTexture(TextureMap *map) { specular.SetTexture(map); }

  void SetEmissionTexture(TextureMap *map) { emission.SetTexture(map); }

  void SetReflectionTexture(TextureMap *map) { reflection.SetTexture(map); }

  void SetRefractionTexture(TextureMap *map) { refraction.SetTexture(map); }

  void SetReflectionGlossiness(float gloss);

  void SetRefractionGlossiness(float gloss);

  Color3f Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                const LightList &lights, int bounceCount) const override;

  // Photon Extensions
  // If this method returns true, the photon will be stored
  bool IsPhotonSurface(int subMtlID) const override
  {
    return ColorLuma(diffuse.GetColor()) > 0;
  }

  // If this method returns true, a new photon with the given direction and
  // color will be traced
  bool RandomPhotonBounce(DiffRay &, Color3f &,
                          const DiffHitInfo &) const override;

  // OpenGL Extensions
  void SetViewportMaterial(int subMtlID) const override;

private:

  bool ComputeFresnel(const DiffRay &, const DiffHitInfo &,
                      Point3& transmitDir, Point3& reflectDir,
                      float& transmitRatio, float& reflectRatio) const;

  enum MtlSelection { TRANSMIT, REFLECT, DIFFUSE, ABSORB };

  MtlSelection RandomSelectMtl(float &scale,
                               const Color3f &sampleTransmission,
                               const Color3f &sampleReflection,
                               const Color3f &sampleDiffuse)
  const;

  bool SampleTransmitBxDF(Point3 &sampleDir,
                          Color3f &BxDF,
                          float &PDF,
                          const Point3 &N,
                          const Point3 &Y,
                          const Point3 &V,
                          const Point3 &tDir,
                          const Color3f &color,
                          bool photonMap = false)
  const;

  bool SampleReflectionBxDF(Point3 &sampleDir,
                            Color3f &BxDF,
                            float &PDF,
                            const Point3 &N,
                            const Point3 &Y,
                            const Point3 &V,
                            const Point3 &rDir,
                            const Color3f &color,
                            bool photonMap = false)
  const;

  bool SampleDiffuseBxDF(Point3 &sampleDir,
                         Color3f &BxDF,
                         float &PDF,
                         const Point3 &N,
                         const Point3 &V,
                         const Color3f &colorDiffuse,
                         const Color3f &colorSpecular,
                         bool photonMap = false)
  const;

  Color3f ComputeSecondaryRay(const Point3& pos,
                              const Point3 &dir,
                              const Color3f &BxDF,
                              const float &pdf,
                              const LightList &lights,
                              int bounceCount,
                              bool hasDiffuseHit = false) const;

private:
  TexturedColor diffuse, specular;
  TexturedColor reflection, refraction;
  TexturedColor emission;
  Color3f absorption;
  float kill;
  float ior; // index of refraction
  float specularGlossiness, reflectionGlossiness, refractionGlossiness;
};
}

#endif //QARAY_MTLBLINN_PHOTON_MAP_H
