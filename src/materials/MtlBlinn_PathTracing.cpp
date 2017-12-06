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
#include "MtlBlinn_PathTracing.h"
#include "lights/lights.h"
#include "materials/materials.h"
//------------------------------------------------------------------------------
namespace qaray {

MtlBlinn_PathTracing::MtlBlinn_PathTracing() :
    diffuse(0.5f, 0.5f, 0.5f),
    specular(0.7f, 0.7f, 0.7f),
    emission(0, 0, 0),
    reflection(0, 0, 0),
    refraction(0, 0, 0),
    absorption(0, 0, 0),
    ior(1),
    specularGlossiness(20.0f),
    reflectionGlossiness(0),
    refractionGlossiness(0) {}

//------------------------------------------------------------------------------

float ColorMax(const Color3f &c) { return MAX(c.x, MAX(c.y, c.z)); }

//------------------------------------------------------------------------------

const float total_reflection_threshold = 1.001f;
const float glossiness_value_threshold = 0.001f;
const float glossiness_power_threshold = 0.f;

//------------------------------------------------------------------------------

void MtlBlinn_PathTracing::SetReflectionGlossiness(float gloss)
{
  reflectionGlossiness = gloss > glossiness_value_threshold ?
                         1.f / gloss : -1.f;
}

void MtlBlinn_PathTracing::SetRefractionGlossiness(float gloss)
{
  refractionGlossiness = gloss > glossiness_value_threshold ?
                         1.f / gloss : -1.f;
}

Color3f MtlBlinn_PathTracing::Shade(const DiffRay &ray,
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
  // X Differential Ray Incoming Direction
  // Y Differential Ray Incoming Direction
  // Surface Position in World Coordinate
  const auto N = normalize(hInfo.c.N);
  const auto V = normalize(-ray.c.dir);
  const auto Vx = normalize(-ray.x.dir);
  const auto Vy = normalize(-ray.y.dir);
  const auto p = hInfo.c.p;
  const auto px = ray.x.p + ray.x.dir * hInfo.x.z;
  const auto py = ray.y.p + ray.y.dir * hInfo.y.z;
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
  const Point3 tDir = -X * sinO - Y * cosO;           // Transmission
  const Point3 rDir = 2.f * N * (dot(N, V)) - V; // Reflection
  //
  // Reflection and Transmission coefficients
  //
  const float C0 = (nIOR - 1.f) * (nIOR - 1.f) / ((nIOR + 1.f) * (nIOR + 1.f));
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  //
  // Reflection and Transmission Colors
  //
  const bool totReflection = (nIOR * sinI) > total_reflection_threshold;
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
  float coefDirectLight = 0.f;
  float coefRefraction = ColorMax(sampleRefraction);
  float coefReflection = ColorMax(sampleReflection);
  float coefSpecular = ColorMax(sampleSpecular);
  float coefDiffuse = ColorMax(sampleDiffuse);
  const float coefSum =
      coefDirectLight + coefRefraction + coefReflection + coefSpecular
          + coefDiffuse;
  coefDirectLight /= coefSum;
  coefRefraction /= coefSum;
  coefReflection /= coefSum;
  coefSpecular /= coefSum;
  coefDiffuse /= coefSum;
  const float sumDirectLight = coefDirectLight;
  const float sumRefraction = coefRefraction + coefDirectLight;
  const float sumReflection = coefReflection + coefRefraction + coefDirectLight;
  const float sumSpecular =
      coefSpecular + coefReflection + coefRefraction + coefDirectLight;
  const float
      sumDiffuse = 1.00001f; /* make sure everything is inside the range */
  //
  // Shading Directional Lights
  //
  const float normCoefDI = (lights.size() == 0 ? 1.f : 1.f / lights.size());
  for (auto &light : lights) {
    if (light->IsAmbient()) {}
    else {
      auto intensity = light->Illuminate(p, N) * normCoefDI;
      auto L = normalize(-light->Direction(p));
      auto H = normalize(V + L);
      auto cosNL = MAX(0.f, dot(N, L));
      auto cosNH = MAX(0.f, dot(N, H));
      color += normCoefDI * intensity * cosNL *
          (sampleDiffuse + sampleSpecular * POW(cosNH, specularGlossiness));
    }
  }
  //
  // Shading Indirectional Lights
  //
  if (bounceCount > 0) {
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
    // Select a BxDF
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
        const Point3 H = tDir;
        const float cosVH = MAX(0.f, dot(V, H));
        const float glossiness = POW(cosVH, refractionGlossiness); // My Hack
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
        PDF = coefReflection;
        /* BRDF */
        const Point3 L = normalize(sampleDir);
        const Point3 H = rDir;
        const float cosVH = MAX(0.f, dot(V, H));
        BxDF = sampleReflection * POW(cosVH, reflectionGlossiness);
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
      if (specularGlossiness > glossiness_power_threshold) {
        if (hInfo.c.hasFrontHit) {
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
    else if (coefDiffuse > 1e-6f) {
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
}
//------------------------------------------------------------------------------
