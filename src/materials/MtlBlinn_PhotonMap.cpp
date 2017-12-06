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
    kill(0),
    ior(1),
    specularGlossiness(20.f),
    reflectionGlossiness(0),
    refractionGlossiness(0) {}
///--------------------------------------------------------------------------//
void MtlBlinn_PhotonMap::SetReflectionGlossiness(float gloss)
{
  reflectionGlossiness = gloss > glossiness_value_threshold ?
                         1.f / gloss : -1.f;
}
void MtlBlinn_PhotonMap::SetRefractionGlossiness(float gloss)
{
  refractionGlossiness = gloss > glossiness_value_threshold ?
                         1.f / gloss : -1.f;
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
  transmitDir = -X * sinO - Y * cosO;      // Transmission
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
                                    const Color3f &sampleDiffuse,
                                    const Color3f &sampleSpecular)
const
{
  //
  // Determine Color Strength
  //
  float lumaTransmission = ColorLuma(sampleTransmission);
  float lumaReflection = ColorLuma(sampleReflection);
  float lumaSpecular = ColorLuma(sampleSpecular);
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
  const float coefSpecular = coefReflection + lumaSpecular;
  const float coefDiffuse = coefSpecular + lumaDiffuse;
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
  } else if (select < coefSpecular && lumaSpecular > color_luma_threshold) {
    selectedMtl = SPECULAR;
    scale = lumaSpecular * rcpCoefSum;
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
    sampleDir = photonMap ?
                -TransformToLocalFrame(Y, rng->local().UniformHemisphere()) :
                -TransformToLocalFrame(Y, rng->local().CosWeightedHemisphere());
    const Point3 L = normalize(sampleDir);
    const Point3 H = normalize(V + L);
    const float cosVH = MAX(0.f, dot(V, H));
    const float glossiness = POW(cosVH, refractionGlossiness); // My Hack
    BxDF = color * glossiness; // ==> rho / pi
    PDF = photonMap ?
          0.5f : // ==> 1 / (2*pi) uniform hemisphere sampling
          1.f;  // ==> cosTheta / pi cos-weighted hemisphere sampling
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
    sampleDir = photonMap ?
                TransformToLocalFrame(Y, rng->local().UniformHemisphere()) :
                TransformToLocalFrame(Y, rng->local().CosWeightedHemisphere());
    const Point3 L = normalize(sampleDir);
    const Point3 H = normalize(V + L);
    const float cosNH = MAX(0.f, dot(N, H));
    BxDF = color * POW(cosNH, reflectionGlossiness);
    PDF = photonMap ?
          0.5f :
          1.f;
  } else {
    sampleDir = rDir;
    BxDF = color;
    PDF = 1.f;
  }
  return true;
}
bool MtlBlinn_PhotonMap::SampleSpecularBxDF(Point3 &sampleDir,
                                            Color3f &BxDF,
                                            float &PDF,
                                            const Point3 &N,
                                            const Point3 &V,
                                            const Color3f &color,
                                            bool photonMap)
const
{
  if (specularGlossiness > glossiness_power_threshold) {
    sampleDir = photonMap ?
                TransformToLocalFrame(N, rng->local().UniformHemisphere()) :
                TransformToLocalFrame(N, rng->local().CosWeightedHemisphere());
    const Point3 L = normalize(sampleDir);
    const Point3 H = normalize(V + L);
    const float cosNH = MAX(0.f, dot(N, H));
    BxDF = color * POW(cosNH, specularGlossiness);
    PDF = photonMap ?
          0.5f :
          1.f;
    return true;
  } else {
    return false;
  }
}
bool MtlBlinn_PhotonMap::SampleDiffuseBxDF(Point3 &sampleDir,
                                           Color3f &BxDF,
                                           float &PDF,
                                           const Point3 &N,
                                           const Color3f &color,
                                           bool photonMap)
const
{
  sampleDir = photonMap ?
              TransformToLocalFrame(N, rng->local().UniformHemisphere()) :
              TransformToLocalFrame(N, rng->local().CosWeightedHemisphere());
  BxDF = color;
  PDF = photonMap ?
        0.5f :
        1.f;
  return true;
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
  // Russian Roulette
  //
  Point3 sampleDir;
  Color3f BxDF;
  qaFLOAT PDF = 1.f;
  qaFLOAT scale = 1.f;
  qaBOOL doShade = false;
  auto select = RandomSelectMtl(scale, sampleTransmission, sampleReflection,
                                sampleDiffuse, sampleSpecular);
  // Generate Parameters
  switch (select) {
    case (TRANSMIT):
      doShade = SampleTransmitBxDF(sampleDir, BxDF, PDF, N, Y, V, tDir,
                                   sampleTransmission);
      break;
    case (REFLECT):
      doShade = SampleReflectionBxDF(sampleDir, BxDF, PDF, N, Y, V, rDir,
                                     sampleReflection);
      break;
    case (SPECULAR):
      if (hInfo.c.hasFrontHit) {
        doShade =
            SampleSpecularBxDF(sampleDir, BxDF, PDF, N, V, sampleSpecular);
      }
      break;
    case (DIFFUSE):
      if (hInfo.c.hasFrontHit) {
        doShade = SampleDiffuseBxDF(sampleDir, BxDF, PDF, N, sampleDiffuse);
      }
      break;
    case (ABSORB): doShade = false;
      break;
  }
  //
  // Shading Directional Lights
  //
  //qaBOOL doDirectLight = true;
  qaBOOL doDirectLight = select != DIFFUSE && select != SPECULAR;
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
        color += intensity * cosNL *
            (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
      }
    }
  }
  //
  // Gather Photon
  //
  //qaBOOL doPhotonGather = select == DIFFUSE &&
  //       (bounceCount != Material::maxBounce || Material::maxBounce == 0);
  qaBOOL doPhotonGather = (select == DIFFUSE || select == SPECULAR);
  if (doPhotonGather) {
    // gather photons
    cyColor cyI;
    cyPoint3f cyD;
    cyPoint3f cyP(p.x, p.y, p.z);
    cyPoint3f cyN(N.x, N.y, N.z);
    const float radius = 1.f;
    scene.photonmap.EstimateIrradiance<400>
        (cyI, cyD, radius, cyP, &cyN, 1.f,
         cyPhotonMap::FILTER_TYPE_QUADRATIC);
    // shade
    Color3f I(cyI.r, cyI.g, cyI.b);
    I *= RCP_PI / (radius * radius);
    Point3 L = -normalize(Point3(cyD.x, cyD.y, cyD.z));
    auto H = normalize(V + L);
    auto cosNL = MAX(0.f, dot(N, L));
    color += I * cosNL * BxDF / PDF;
  }
  //
  // Shading Indirectional Lights
  //
  if (bounceCount > 0 && !doPhotonGather) // Select BxDF
  {
    if (doShade) {
      // Generate ray
      DiffRay sampleRay(p, sampleDir);
      sampleRay.Normalize();
      DiffHitInfo sampleHInfo;
      sampleHInfo.Init();
      // Integrate Incoming Ray
      Color3f incoming(0.f);
      if (scene.TraceNodeNormal(scene.rootNode, sampleRay, sampleHInfo)) {
        // Attenuation When the Ray Travels Inside the Material
        if (!sampleHInfo.c.hasFrontHit) {
          incoming *= Attenuation(absorption, sampleHInfo.c.z);
        }
        const auto *mtl = sampleHInfo.c.node->GetMaterial();
        incoming =
            mtl->Shade(sampleRay, sampleHInfo, lights, bounceCount - 1);
      } else {
        incoming = scene.environment.SampleEnvironment(sampleRay.c.dir);
      }
      Point3 outgoing = incoming * BxDF / (PDF * scale);
      color += outgoing;
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
  const Color3f sampleSpecular = Sample(hInfo, specular);
  const Color3f sampleDiffuse = Sample(hInfo, diffuse);
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
                                sampleDiffuse,
                                sampleSpecular);
  switch (select) {
    case (TRANSMIT):
      doShade = SampleTransmitBxDF(sampleDir,
                                   BxDF,
                                   PDF,
                                   N,
                                   Y,
                                   V,
                                   tDir,
                                   sampleTransmission,
                                   true);
      break;
    case (REFLECT):
      doShade = SampleReflectionBxDF(sampleDir,
                                     BxDF,
                                     PDF,
                                     N,
                                     Y,
                                     V,
                                     rDir,
                                     sampleReflection,
                                     true);
      break;
    case (SPECULAR):
      if (hInfo.c.hasFrontHit) {
        doShade =
            SampleSpecularBxDF(sampleDir,
                               BxDF,
                               PDF,
                               N,
                               V,
                               sampleSpecular,
                               true);
      }
      break;
    case (DIFFUSE):
      if (hInfo.c.hasFrontHit) {
        doShade =
            SampleDiffuseBxDF(sampleDir, BxDF, PDF, N, sampleDiffuse, true);
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
    return true;
  } else {
    return false;
  }
}
};
//------------------------------------------------------------------------------
