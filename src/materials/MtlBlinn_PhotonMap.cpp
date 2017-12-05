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
    ior(1),
    specularGlossiness(20.f),
    reflectionGlossiness(20.f),
    refractionGlossiness(0)
{}
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
//------------------------------------------------------------------------------
bool MtlBlinn_PhotonMap::ComputeFresnel(const DiffRay &ray,
                                        const DiffHitInfo &hInfo,
                                        Point3& transmitDir,
                                        Point3& reflectDir,
                                        float& transmitRatio,
                                        float& reflectRatio)
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
  const auto Y = dot(N, V) > 0.f ? N : -N;    // Vy
  const auto Z = cross(V, Y);
  const auto X = normalize(cross(Y, Z)); // Vx
  //
  // Index of Refraction
  //
  const float nIOR = hInfo.c.hasFrontHit ? 1.f / ior : ior;
  const float cosI = dot(N, V);
  const float sinI = SQRT(1 - cosI * cosI);
  const float sinO = MAX(0.f, MIN(1.f, sinI * nIOR));
  const float cosO = SQRT(1.f - sinO * sinO);
  transmitDir = -X * sinO - Y * cosO;      // Transmission
  reflectDir  = 2.f * N * (dot(N, V)) - V; // Reflection
  //
  // Reflection and Transmission Coefficients
  //
  const float C = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  reflectRatio = C + (1.f - C) * POW(1.f - ABS(cosI), 5.f); // reflection
  transmitRatio = 1.f - reflectRatio;
  return (nIOR * sinI) > total_reflection_threshold;
}

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
  const auto N = hInfo.c.N;
  const auto V = -ray.c.dir;
  const auto p = hInfo.c.p;
  //
  // Local Coordinate Frame
  //
  const auto Y = dot(N, V) > 0.f ? N : -N; // Vy
  const auto Z = cross(V, Y);
  const auto X = normalize(cross(Y, Z));   // Vx
  //
  //
  //
  Point3 tDir, rDir; float tC, rC;
  const bool totReflection = ComputeFresnel(ray, hInfo, tDir, rDir, tC, rC);
  //
  // Reflection and Transmission Colors
  //
  const Color3f tK =
      hInfo.c.hasTexture ?
      refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
  const Color3f rK =
      hInfo.c.hasTexture ?
      reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
  const Color3f sampleRefraction = totReflection ? Color3f(0.f) : tK * tC;
  const Color3f sampleReflection = totReflection ? (rK + tK) : (rK + tK * rC);
  const Color3f sampleDiffuse =
      hInfo.c.hasTexture ?
      diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      diffuse.GetColor();
  const Color3f sampleSpecular =
      hInfo.c.hasTexture ?
      specular.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      specular.GetColor();
  //
  // Throw A Dice
  //
  float select;
  rng->local().Get1f(select);
  //
  // Compute weights
  //
  float coefRefraction = ColorLuma(sampleRefraction);
  float coefReflection = ColorLuma(sampleReflection);
  float coefSpecular = ColorLuma(sampleSpecular);
  float coefDiffuse  = ColorLuma(sampleDiffuse);
  const float coefSum1 = coefRefraction + coefReflection;
  const float coefSum2 = coefDiffuse + coefSpecular + coefRefraction + coefReflection;
  coefRefraction /= hInfo.c.hasFrontHit ? coefSum2 : (coefSum1 > 0.f ? coefSum1 : 1.f);
  coefReflection /= hInfo.c.hasFrontHit ? coefSum2 : (coefSum1 > 0.f ? coefSum1 : 1.f);
  coefSpecular   /= coefSum2;
  coefDiffuse    /= coefSum2;
  const float sumRefraction = coefRefraction;
  const float sumReflection = coefReflection + coefRefraction;
  const float sumSpecular   = hInfo.c.hasFrontHit ? coefSpecular + sumReflection : -1.f;
  const float sumDiffuse    = hInfo.c.hasFrontHit ? 1.f : -1.f; /* make sure everything is inside the range */
  //
  // Shading Directional Lights
  //
//  const float normCoefDI = (lights.empty() ? 1.f : 1.f / lights.size()) * RCP_PI;
//  for (auto &light : lights) {
//    if (light->IsAmbient()) {}
//    else {
//      auto intensity = light->Illuminate(p, N) * normCoefDI;
//      auto L = normalize(-light->Direction(p));
//      auto H = normalize(V + L);
//      auto cosNL = MAX(0.f, dot(N, L));
//      auto cosNH = MAX(0.f, dot(N, H));
//      color += normCoefDI * intensity * cosNL *
//          (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
//    }
//  }
  //
  // gather photons
  //
  if (bounceCount <= 0) {
    cyColor irrad;
    cyPoint3f direction;
    cyPoint3f cypos(p.x, p.y, p.z);
    cyPoint3f cyNor(N.x, N.y, N.z);
    const float radius = 1.f;
    scene.photonmap.EstimateIrradiance<2000>
        (irrad, direction, radius, cypos, &cyNor, 1.f, cyPhotonMap::FILTER_TYPE_QUADRATIC);
    // shade
    Color3f intensity(irrad.r, irrad.g, irrad.b);
    intensity *= RCP_PI / length2(radius);

    Point3 L = normalize(Point3(direction.x, direction.y, direction.z));
    auto H = normalize(V + L);
    auto cosNL = MAX(0.f, dot(N, L));
    auto cosNH = MAX(0.f, dot(N, H));
    color += intensity * cosNL *
        (sampleDiffuse /*+ sampleSpecular * POW(cosNH, specularGlossiness)*/);

    color += intensity;
  }
  //
  // Shading Indirectional Lights
  //
  else if (bounceCount > 0) {
    //
    // Coordinate Frame for the Hemisphere
    const Point3 nZ = Y;
    const Point3 nY = (ABS(nZ.x) > ABS(nZ.y)) ?
                      normalize(Point3(nZ.z, 0, -nZ.x)) :
                      normalize(Point3(0, -nZ.z, nZ.y));
    const Point3 nX = normalize(cross(nY, nZ));
    Point3 sampleDir(0.f);
    bool doShade = false;
    //
    // Select a BxDF & PDF
    //
    float PDF = 1.f;
    Color3f BxDF(0.f);
    /* Refraction */
    if (select <= sumRefraction && coefRefraction > 1e-6f) {
      if (refractionGlossiness > glossiness_power_threshold) {
        /* Random Sampling for Glossy Surface */
        const Point3
            sample = normalize(rng->local().CosWeightedHemisphere());
        sampleDir = -(sample.x * nX + sample.y * nY + sample.z * nZ);
        /* PDF */
        PDF = coefRefraction;
        /* BSDF */
        const Point3 L = normalize(sampleDir);
        const Point3 H = normalize(V + L);
        const float cosNH = MAX(0.f, dot(N, H));
        const float glossiness = POW(cosNH, refractionGlossiness); // My Hack
        BxDF = sampleRefraction * glossiness;
      } else {
        /* Ray Direction */
        sampleDir = tDir;
        /* PDF */
        PDF = coefRefraction;
        /* BSDF */
        BxDF = sampleRefraction;
      }
      doShade = true;
    }
      /* Reflection */
    else if (select < sumReflection && coefReflection > 1e-6f) {
      if (reflectionGlossiness > glossiness_power_threshold) {
        /* Random Sampling for Glossy Surface */
        const Point3
            sample = normalize(rng->local().CosWeightedHemisphere());
        sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
        /* PDF */
        PDF = coefReflection * RCP_PI;
        /* BRDF */
        const Point3 L = normalize(sampleDir);
        const Point3 H = normalize(V + L);
        const float cosNH = MAX(0.f, dot(N, H));
        BxDF = sampleReflection * POW(cosNH, reflectionGlossiness);
      } else {
        /* Ray Direction */
        sampleDir = rDir;
        /* PDF */
        PDF = coefReflection;
        /* BRDF */
        BxDF = sampleReflection;
      }
      doShade = true;
    }
      /* Specular */
    else if (select < sumSpecular && coefSpecular > 1e-6f) {
      if (hInfo.c.hasFrontHit) {
        if (specularGlossiness > glossiness_power_threshold) {
          /* Random Sampling for Glossy Surface */
          const Point3
              sample = normalize(rng->local().CosWeightedHemisphere());
          sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
          /* PDF */
          PDF = coefSpecular;
          /* BRDF */
          const Point3 L = normalize(sampleDir);
          const Point3 H = normalize(V + L);
          const float cosNH = MAX(0.f, dot(N, H));
          BxDF = sampleSpecular * POW(cosNH, specularGlossiness);
          doShade = true;
        }
      }
    }
      /* Diffuse */
    else if (select < sumDiffuse && coefDiffuse > 1e-6f) {
      if (hInfo.c.hasFrontHit) {
        /* Generate Random Sample */
        const Point3
            sample = normalize(rng->local().CosWeightedHemisphere());
        sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
        /* PDF */
        PDF = coefDiffuse;
        /* BRDF */
        BxDF = sampleDiffuse;
        doShade = true;
      }
    }
    //
    // Shade
    //
    if (doShade) {
      // Generate ray
      DiffRay sampleRay(p, sampleDir);
      DiffHitInfo sampleHInfo;
      sampleHInfo.c.z = BIGFLOAT;
      sampleRay.Normalize();
      // Integrate Incoming Ray
      Color3f incoming(0.f);
      if (scene.TraceNodeNormal(scene.rootNode, sampleRay, sampleHInfo)) {
        // Attenuation When the Ray Travels Inside the Material
        if (!sampleHInfo.c.hasFrontHit) {
          incoming *= Attenuation(absorption, sampleHInfo.c.z);
        }
        const auto *mtl = sampleHInfo.c.node->GetMaterial();
        incoming = mtl->Shade(sampleRay, sampleHInfo, lights, bounceCount - 1);
      } else {
        incoming = scene.environment.SampleEnvironment(sampleRay.c.dir);
      }
      Point3 outgoing = incoming * BxDF / PDF;
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
  // Surface Normal In World Coordinate
  // Ray Incoming Direction
  // Surface Position in World Coordinate
  const auto N = hInfo.c.N;
  const auto V = -ray.c.dir;
  const auto p = hInfo.c.p;
  //
  // Local Coordinate Frame
  //
  const auto Y = dot(N, V) > 0.f ? N : -N; // Vy
  const auto Z = cross(V, Y);
  const auto X = normalize(cross(Y, Z));   // Vx
  //
  //
  //
  Point3 tDir, rDir; float tC, rC;
  const bool totReflection = ComputeFresnel(ray, hInfo, tDir, rDir, tC, rC);
  //
  // Reflection and Transmission Colors
  //
  const Color3f tK =
      hInfo.c.hasTexture ?
      refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
  const Color3f rK =
      hInfo.c.hasTexture ?
      reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
  const Color3f sampleRefraction = totReflection ? Color3f(0.f) : tK * tC;
  const Color3f sampleReflection = totReflection ? (rK + tK) : (rK + tK * rC);
  const Color3f sampleDiffuse =
      hInfo.c.hasTexture ?
      diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      diffuse.GetColor();
  const Color3f sampleSpecular =
      hInfo.c.hasTexture ?
      specular.Sample(hInfo.c.uvw, hInfo.c.duvw) :
      specular.GetColor();
  //
  //
  //
  float select;
  rng->local().Get1f(select);
  float coefRefraction = ColorLuma(sampleRefraction);
  float coefReflection = ColorLuma(sampleReflection);
  float coefSpecular = ColorLuma(sampleSpecular);
  float coefDiffuse  = ColorLuma(sampleDiffuse);
  const float coefSum = coefRefraction + coefReflection + coefSpecular + coefDiffuse;
  coefRefraction /= coefSum;
  coefReflection /= coefSum;
  coefSpecular /= coefSum;
  coefDiffuse /= coefSum;
  const float sumRefraction = coefRefraction;
  const float sumReflection = sumRefraction + coefReflection;
  const float sumSpecular = sumReflection + coefSpecular;
  const float sumDiffuse = 1.00001f; /* make sure everything is inside the range */
  bool useRefraction = select < sumRefraction && coefRefraction > 1e-6f;
  bool useReflection = select < sumReflection && coefReflection > 1e-6f;
  bool useSpecular   = select < sumSpecular && coefSpecular > 1e-6f;
  bool useDiffuse    = select < sumDiffuse && coefDiffuse > 1e-6f;
  //
  //
  // Coordinate Frame for the Hemisphere
  //
  const Point3 nZ = Y;
  const Point3 nY = (ABS(nZ.x) > ABS(nZ.y)) ?
                    normalize(Point3(nZ.z, 0, -nZ.x)) :
                    normalize(Point3(0, -nZ.z, nZ.y));
  const Point3 nX = normalize(cross(nY, nZ));
  Point3 sampleDir(0.f);
  Color3f BxDF(0.f);
  float PDF = 1.f;
  bool doShade = false;
  /* Refraction */
  if (useRefraction) {
    if (refractionGlossiness > glossiness_power_threshold) {
      /* Random Sampling for Glossy Surface */
      const Point3 sample = normalize(rng->local().UniformHemisphere());
      sampleDir = -(sample.x * nX + sample.y * nY + sample.z * nZ);
      /* BSDF */
      const Point3 L = normalize(sampleDir);
      const Point3 H = normalize(V + L);
      const float cosNH = MAX(0.f, dot(N, H));
      const float glossiness = POW(cosNH, refractionGlossiness); // My Hack
      BxDF = sampleRefraction * glossiness;
      PDF = coefRefraction;
    } else {
      /* Ray Direction */
      sampleDir = tDir;
      /* BSDF */
      BxDF = sampleRefraction;
      PDF = coefRefraction;
    }
    doShade = true;
  }
  /* Reflection */
  else if (useReflection) {
    if (reflectionGlossiness > glossiness_power_threshold) {
      /* Random Sampling for Glossy Surface */
      const Point3 sample = normalize(rng->local().UniformHemisphere());
      sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
      /* BRDF */
      const Point3 L = normalize(sampleDir);
      const Point3 H = normalize(V + L);
      const float cosNH = MAX(0.f, dot(N, H));
      BxDF = sampleReflection * POW(cosNH, reflectionGlossiness);
      PDF = coefReflection;
    } else {
      /* Ray Direction */
      sampleDir = rDir;
      /* BRDF */
      BxDF = sampleReflection;
      PDF = coefReflection;
    }
    doShade = true;
  }
  /* Specular */
  else if (useSpecular) {
    if (specularGlossiness > glossiness_power_threshold) {
      if (hInfo.c.hasFrontHit) {
        /* Random Sampling for Glossy Surface */
        const Point3
            sample = normalize(rng->local().UniformHemisphere());
        sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
        /* BRDF */
        const Point3 L = normalize(sampleDir);
        const Point3 H = normalize(V + L);
        const float cosNH = MAX(0.f, dot(N, H));
        BxDF = sampleSpecular * POW(cosNH, specularGlossiness);
        PDF = coefSpecular;
        doShade = true;
      }
    }
  }
  /* Diffuse */
  else if (useDiffuse) {
    if (hInfo.c.hasFrontHit) {
      /* Generate Random Sample */
      const Point3 sample = normalize(rng->local().UniformHemisphere());
      sampleDir = sample.x * nX + sample.y * nY + sample.z * nZ;
      /* BRDF */
      BxDF = sampleDiffuse;
      PDF = coefDiffuse;
      doShade = true;
    }
  }
  if (doShade)
  {
    ray = DiffRay(hInfo.c.p, sampleDir); ray.Normalize();
    c = c * 2.f * BxDF / PDF;
    return true;
  }
  else {
    return false;
  }
}
};
//------------------------------------------------------------------------------
