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
#include "MtlBlinn_PhotonMap.h"
#include "materials/materials.h"
///--------------------------------------------------------------------------//
const float total_reflection_threshold = 1.001f;
const float glossiness_value_threshold = 0.001f;
const float glossiness_power_threshold = 0.f;
const float color_luma_threshold = 0.00001f;
static Color3f Sample(const DiffHitInfo &hInfo, const TexturedColor &c)
{
  return hInfo.c.hasTexture ?
         c.Sample(hInfo.c.uvw, hInfo.c.duvw) :
         c.GetColor();
}
///--------------------------------------------------------------------------//
namespace qaray {
///--------------------------------------------------------------------------//
MtlBlinn_PhotonMap::MtlBlinn_PhotonMap() :
    diffuse(0.5f, 0.5f, 0.5f),
    specular(0.7f, 0.7f, 0.7f),
    emission(0, 0, 0),
    reflection(0, 0, 0),
    refraction(0, 0, 0),
    absorption(0, 0, 0),
    kill(0.1),
    ior(1),
    specularGlossiness(20.f),
    reflectionGlossiness(0),
    refractionGlossiness(0) {}
///--------------------------------------------------------------------------//
void MtlBlinn_PhotonMap::SetReflectionGlossiness(float gloss)
{
  reflectionGlossiness = gloss > glossiness_value_threshold ? gloss : -1.f;
}
void MtlBlinn_PhotonMap::SetRefractionGlossiness(float gloss)
{
  refractionGlossiness = gloss > glossiness_value_threshold ? gloss : -1.f;
}
//---------------------------------------------------------------------------//
bool MtlBlinn_PhotonMap::ComputeFresnel(const DiffRay &ray,
                                        const DiffHitInfo &hInfo,
                                        Point3 &transmitDir,
                                        Point3 &reflectDir,
                                        float &transmitRatio,
                                        float &reflectRatio)
const
{
  // Surface Normal In World Coordinate
  // Ray Incoming Direction
  // X Differential Ray Incoming Direction
  // Y Differential Ray Incoming Direction
  // Surface Position in World Coordinate
  const auto N = hInfo.c.N;
  const auto V = -ray.c.dir;
  const auto p = hInfo.c.p;
  //
  // Local Coordinate Frame
  //
  const auto Y = dot(N, V) > 0.f ? N : -N;
  const auto Z = cross(V, Y);
  const auto X = normalize(cross(Y, Z));
  //
  // Index of Refraction
  //
  const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;
  const float cosI = dot(N, V);
  const float sinI = SQRT(1 - cosI * cosI);
  const float sinO = MAX(0.f, MIN(1.f, sinI * nIOR));
  const float cosO = SQRT(1.f - sinO * sinO);
  transmitDir = -X * sinO - Y * cosO;     // Transmission
  reflectDir = 2.f * N * (dot(N, V)) - V; // Reflection
  //
  // Reflection and Transmission Coefficients
  //
  const bool totalReflection = (nIOR * sinI) > total_reflection_threshold;
  const float C = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  reflectRatio = C + (1.f - C) * POW(1.f - ABS(cosI), 5.f); // reflection
  transmitRatio = 1.f - reflectRatio;
  return totalReflection;
}
///--------------------------------------------------------------------------//
MtlBlinn_PhotonMap::MtlSelection
MtlBlinn_PhotonMap::RandomSelectMtl(float &scale,
                                    const Color3f &sampleTransmission,
                                    const Color3f &sampleReflection,
                                    const Color3f &sampleDiffuse)
const
{
  //
  // Determine Color Strength
  //
  float lumaTransmission = ColorLuma(sampleTransmission);
  float lumaReflection = ColorLuma(sampleReflection);
  float lumaDiffuse = ColorLuma(sampleDiffuse);
  //
  // Throw A Dice
  //
  float r;
  rng->local().Get1f(r);
  //
  // Compute weights
  //
  const float coefTransmit = lumaTransmission;
  const float coefReflection = coefTransmit + lumaReflection;
  const float coefDiffuse = coefReflection + lumaDiffuse;
  const float coefAbsorb = coefDiffuse + kill;
  const float coefSum = coefAbsorb;
  const float rcpCoefSum = 1.f / coefSum;
  const float select = r * coefSum;
  MtlSelection selectedMtl;
  if (select < coefTransmit && lumaTransmission > color_luma_threshold) {
    selectedMtl = TRANSMIT;
    scale = lumaTransmission * rcpCoefSum;
  } else if (select < coefReflection && lumaReflection > color_luma_threshold) {
    selectedMtl = REFLECT;
    scale = lumaReflection * rcpCoefSum;
  } else if (select < coefDiffuse && lumaDiffuse > color_luma_threshold) {
    selectedMtl = DIFFUSE;
    scale = lumaDiffuse * rcpCoefSum;
  } else {
    selectedMtl = ABSORB;
    scale = coefAbsorb * rcpCoefSum;
  }
  return selectedMtl;
}
///--------------------------------------------------------------------------//
bool MtlBlinn_PhotonMap::SampleTransmitBxDF(Point3 &sampleDir,
                                            Color3f &BxDF,
                                            float &PDF,
                                            const Point3 &N,
                                            const Point3 &Y,
                                            const Point3 &V,
                                            const Point3 &tDir,
                                            const Color3f &color,
                                            bool photonMap)
const
{
  if (refractionGlossiness > glossiness_power_threshold) {
    do {
      sampleDir = normalize(normalize(tDir) + rng->local().UniformBall(refractionGlossiness));
    } while (dot(sampleDir, Y) > 0);
    const Point3 L = normalize(sampleDir);
    const auto cosNL = MAX(0.f, dot(N, L));
    BxDF = photonMap ? color : color * cosNL; // ==> rho / pi
    // compute sinTheta
    const float y0 = SQRT(1.f / (refractionGlossiness * refractionGlossiness + 1.f));
    const float y1 = SQRT(1 - cosNL * cosNL);
    PDF = 0.5f / (1.f - MAX(y0, y1)); // 1 / (2*pi*sinTheta)
  } else {
    sampleDir = tDir;
    BxDF = color; // ==> reflect all light
    PDF = 1.f;    // ==> PDF = 1
  }
  return true;
}
bool MtlBlinn_PhotonMap::SampleReflectionBxDF(Point3 &sampleDir,
                                              Color3f &BxDF,
                                              float &PDF,
                                              const Point3 &N,
                                              const Point3 &Y,
                                              const Point3 &V,
                                              const Point3 &rDir,
                                              const Color3f &color,
                                              bool photonMap)
const
{
  if (reflectionGlossiness > glossiness_power_threshold) {
    do {
      sampleDir = normalize(normalize(rDir) + rng->local().UniformBall(reflectionGlossiness));
    } while (dot(sampleDir, Y) < 0);
    //const Point3 L = normalize(sampleDir);
    //const auto cosNL = MAX(0.f, dot(N, L));
    //BxDF = photonMap ? color : (color * cosNL);
    BxDF = color;
    //const float y0 = SQRT(1.f / (reflectionGlossiness * reflectionGlossiness + 1.f));
    //const float y1 = SQRT(1 - cosNL * cosNL);
    PDF = 1.f; //0.5f / (1.f - MAX(y0, y1));
  } else {
    sampleDir = rDir;
    BxDF = color;
    PDF = 1.f;
  }
  return true;
}
bool MtlBlinn_PhotonMap::SampleDiffuseBxDF(Point3 &sampleDir,
                                           Color3f &BxDF,
                                           float &PDF,
                                           const Point3 &N,
                                           const Point3 &V,
                                           const Color3f &colorDiffuse,
                                           const Color3f &colorSpecular,
                                           bool photonMap)
const
{
  sampleDir = photonMap ?
              TransformToLocalFrame(N, rng->local().UniformHemisphere()) :
              TransformToLocalFrame(N, rng->local().CosWeightedHemisphere());
  const Point3 L = normalize(sampleDir);
  const Point3 H = normalize(V + L);
  const float cosNH = MAX(0.f, dot(N, H));
  BxDF = colorDiffuse + colorSpecular * POW(cosNH, specularGlossiness);
  PDF = photonMap ?
        0.5f :
        1.f;
  return true;
}
///--------------------------------------------------------------------------//
Color3f MtlBlinn_PhotonMap::ComputeSecondaryRay(const Point3 &pos,
                                                const Point3 &dir,
                                                const Color3f &BxDF,
                                                const float &PDF,
                                                const LightList &lights,
                                                int bounceCount,
                                                bool hasDiffuseHit) const
{
  // Generate ray
  DiffRay sampleRay(pos, dir);
  sampleRay.Normalize();
  DiffHitInfo sampleHInfo;
  sampleHInfo.Init();
  sampleHInfo.c.hasDiffuseHit = hasDiffuseHit;
  // Integrate Incoming Ray
  Color3f incoming(0.f);
  if (scene.TraceNodeNormal(scene.rootNode, sampleRay, sampleHInfo)) {
    const auto *mtl = sampleHInfo.c.node->GetMaterial();
    incoming = mtl->Shade(sampleRay, sampleHInfo, lights, bounceCount - 1);
    // Attenuation When the Ray Travels Inside the Material
    if (!sampleHInfo.c.hasFrontHit) {
      incoming *= Attenuation(absorption, sampleHInfo.c.z);
    }
  } else {
    incoming = scene.environment.SampleEnvironment(sampleRay.c.dir);
  }
  Point3 outgoing = incoming * BxDF / PDF;
  return outgoing;
}
///--------------------------------------------------------------------------//
Color3f MtlBlinn_PhotonMap::Shade(const DiffRay &ray,
                                  const DiffHitInfo &hInfo,
                                  const LightList &lights,
                                  int bounceCount)
const
{
  //
  // Differential Geometry
  //
  Color3f color = hInfo.c.hasTexture ?
                  emission.Sample(hInfo.c.uvw, hInfo.c.duvw) :
                  emission.GetColor();
  // Surface Normal In World Coordinate
  // Ray Incoming Direction
  // Surface Position in World Coordinate
  const auto V = -ray.c.dir;
  const auto N = hInfo.c.N;
  const auto Y = dot(N, V) > 0.f ? N : -N;
  const auto p = hInfo.c.p;
  //
  // Reflection and Transmission Colors
  //
  Point3 tDir, rDir;
  float tC, rC;
  const bool totReflection = ComputeFresnel(ray, hInfo, tDir, rDir, tC, rC);
  const Color3f tK = Sample(hInfo, refraction);
  const Color3f rK = Sample(hInfo, reflection);
  const Color3f sampleTransmission = totReflection ? Color3f(0.f) : tK * tC;
  const Color3f sampleReflection = totReflection ? (rK + tK) : (rK + tK * rC);
  const Color3f sampleSpecular = Sample(hInfo, specular);
  const Color3f sampleDiffuse = Sample(hInfo, diffuse);
  //
  // Other Rays
  //
  if (bounceCount > 0) {
    //
    // Reflect
    //
    if (ColorLuma(sampleReflection) > color_luma_threshold) {
      Point3 sampleDir;
      Color3f BxDF;
      qaFLOAT PDF = 1.f;
      qaBOOL doShade = SampleReflectionBxDF(sampleDir, BxDF, PDF, N, Y, V, rDir,
                                            sampleReflection);
      if (doShade) {
        color += ComputeSecondaryRay(p,
                                     sampleDir,
                                     BxDF,
                                     PDF,
                                     lights,
                                     bounceCount);
      }

    }
    //
    // Transmission
    //
    if (ColorLuma(sampleTransmission) > color_luma_threshold) {
      Point3 sampleDir;
      Color3f BxDF;
      qaFLOAT PDF = 1.f;
      qaBOOL doShade = SampleTransmitBxDF(sampleDir, BxDF, PDF, N, Y, V, tDir,
                                          sampleTransmission);

      if (doShade) {
        color += ComputeSecondaryRay(p,
                                     sampleDir,
                                     BxDF,
                                     PDF,
                                     lights,
                                     bounceCount);
      }

    }
  }
  //
  // Shading Method
  //
  qaBOOL doGatherPhoton = false;
  qaBOOL doGatherCaustics = false;
  qaBOOL doMCSample = false;
  qaBOOL doDirectLight = false;
  // // Method 1:
  qaUINT MCSample = 5;
  doDirectLight = true;
  if (ColorLuma(sampleDiffuse) > color_luma_threshold)
  {
    if (scene.usePhotonMap) {
      if (hInfo.c.hasDiffuseHit) {
        doGatherPhoton = true;
      } else if (bounceCount > 0) {
        doMCSample = true;
      }
      doGatherCaustics = true;
    }
    else
    {
      if (bounceCount > 0) {
        if (hInfo.c.hasDiffuseHit) { MCSample = 1; }
        doMCSample = true;
      }
      doGatherPhoton = false;
      doGatherCaustics = false;
    }
  }
  // // Method 2
  // qaUINT MCSample = 5;
  // doDirectLight = true;
  // if (ColorLuma(sampleDiffuse) > color_luma_threshold)
  // {
  //   doGatherPhoton = true;
  //   doMCSample = false;
  //   doGatherCaustics = true;
  // }
  // // Method 3
  // qaUINT MCSample;
  // doDirectLight = true;
  // if (ColorLuma(sampleDiffuse) > color_luma_threshold)
  // {
  //   if (hInfo.c.hasDiffuseHit) {
  //     MCSample = 1;
  //   } else if (bounceCount > 0) {
  //     MCSample = 10;
  //   }
  //   doGatherPhoton = false;
  //   doMCSample = true;
  //   if (scene.usePhotonMap) {
  //     doGatherCaustics = true;
  //   } else {
  //     doGatherCaustics = false;
  //   }
  // }
  //
  // Gather Photon
  //
  if (doGatherPhoton) {
    Color3f I;
    Point3  D;
    scene.photonmap.map.EstimateIrradiance<100>
        (I, D, scene.photonmap.radius, p, &N, 1.f,
         cyPhotonMap::FILTER_TYPE_QUADRATIC);
    if (ColorLuma(I) > color_luma_threshold) { // in case we found nothing
      const auto L = -normalize(D);
      const auto H = normalize(V + L);
      const auto cosNL = MAX(0.f, dot(N, L));
      const auto cosNH = MAX(0.f, dot(N, H));
      color += I * cosNL *
          (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
    }
  }
  //
  // Gather Caustics
  //
  if (doGatherCaustics)
  {
    Color3f I;
    Point3  D;
    scene.causticsmap.map.EstimateIrradiance<100>
        (I, D, scene.causticsmap.radius, p, &N, 1.f,
         cyPhotonMap::FILTER_TYPE_QUADRATIC);
    if (ColorLuma(I) > color_luma_threshold) { // in case we found nothing
      const auto L = -normalize(D);
      const auto H = normalize(V + L);
      const auto cosNL = MAX(0.f, dot(N, L));
      const auto cosNH = MAX(0.f, dot(N, H));
      color += I * cosNL *
          (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
    }
  }
  //
  // MC Integration
  //
  if (doMCSample) {
    for (size_t i = 0; i < MCSample; ++i) {
      if (hInfo.c.hasFrontHit) {
        Point3 sampleDir;
        Color3f BxDF;
        qaFLOAT PDF = 1.f;
        qaBOOL doShade = SampleDiffuseBxDF(sampleDir, BxDF, PDF, N, V,
                                           sampleDiffuse, sampleSpecular);
        if (doShade) {
          color += 1.f / MCSample * ComputeSecondaryRay(p, sampleDir, BxDF,
                                                        PDF, lights,
                                                        bounceCount, true);
        }
      }
    }
  }
  //
  // Shading Directional Lights
  //
  if (doDirectLight) {
    const float normCoefDI = (lights.empty() ? 1.f : 1.f / lights.size());
    for (auto &light : lights) {
      if (light->IsAmbient()) {}
      else {
        auto intensity = light->Illuminate(p, N) * normCoefDI;
        auto L = normalize(-light->Direction(p));
        auto H = normalize(V + L);
        auto cosNL = MAX(0.f, dot(N, L));
        auto cosNH = MAX(0.f, dot(N, H));
        color += intensity * cosNL
            * (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
      }
    }
  }
  return color;
}
// If this method returns true, a new photon with the given direction and
// color will be traced
bool MtlBlinn_PhotonMap::RandomPhotonBounce(DiffRay &ray, Color3f &c,
                                            const DiffHitInfo &hInfo)
const
{
  //
  // Differential Geometry
  //
  Color3f color = hInfo.c.hasTexture ?
                  emission.Sample(hInfo.c.uvw, hInfo.c.duvw) :
                  emission.GetColor();
  // Surface Normal In World Coordinate
  // Ray Incoming Direction
  // Surface Position in World Coordinate
  const auto V = -ray.c.dir;
  const auto N = hInfo.c.N;
  const auto Y = dot(N, V) > 0.f ? N : -N;
  const auto p = hInfo.c.p;
  //
  // Reflection and Transmission Colors
  //
  Point3 tDir, rDir;
  float tC, rC;
  const bool totReflection = ComputeFresnel(ray, hInfo, tDir, rDir, tC, rC);
  const Color3f tK = Sample(hInfo, refraction);
  const Color3f rK = Sample(hInfo, reflection);
  const Color3f sampleTransmission = totReflection ? Color3f(0.f) : tK * tC;
  const Color3f sampleReflection = totReflection ? (rK + tK) : (rK + tK * rC);
  const Color3f sampleDiffuse = Sample(hInfo, diffuse);
  const Color3f sampleSpecular = Sample(hInfo, specular);
  //
  // Select a BxDF
  //
  Point3 sampleDir;
  Color3f BxDF;
  float PDF = 1.f;
  float scale = 1.f;
  bool doShade = false;
  auto select = RandomSelectMtl(scale,
                                sampleTransmission,
                                sampleReflection,
                                sampleDiffuse);
  switch (select) {
    case (TRANSMIT):
      doShade = SampleTransmitBxDF(sampleDir, BxDF, PDF, N, Y, V, tDir,
                                   sampleTransmission, true);
      break;
    case (REFLECT):
      doShade = SampleReflectionBxDF(sampleDir, BxDF, PDF, N, Y, V, rDir,
                                     sampleReflection, true);
      break;
    case (DIFFUSE):
      if (hInfo.c.hasFrontHit) {
        doShade =
            SampleDiffuseBxDF(sampleDir, BxDF, PDF, N, V,
                              sampleDiffuse, sampleSpecular, true);
      }
      break;
    case (ABSORB):doShade = false;
      break;
  }
  //
  // Color the Photon
  //
  if (doShade) {
    ray = DiffRay(hInfo.c.p, sampleDir);
    ray.Normalize();
    c = c * BxDF / (PDF * scale);
    if (!hInfo.c.hasFrontHit) { c *= Attenuation(absorption, hInfo.c.z); }
    return true;
  } else {
    return false;
  }
}
};
//------------------------------------------------------------------------------
