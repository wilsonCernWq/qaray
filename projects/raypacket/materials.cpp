//------------------------------------------------------------------------------
///
/// \file       materials.cpp
/// \author     Qi WU
/// \version    1.0
/// \date       September, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "materials.h"
#include "globalvar.h"

extern Node rootNode;

//------------------------------------------------------------------------------

Color Attenuation(const Color& absorption, const float l) {
  const float R = exp(-absorption.r * l);
  const float G = exp(-absorption.g * l);
  const float B = exp(-absorption.b * l);
  return Color(R, G, B); // attenuation
}

//------------------------------------------------------------------------------

Color MtlBlinn::Shade(const DiffRay &ray,
		      const DiffHitInfo &hInfo,
		      const LightList &lights,
		      int bounceCount)
  const
{
  // input parameters
  Color color(0.f,0.f,0.f);
  const auto N = glm::normalize(hInfo.c.N);   // surface normal in world coordinate
  const auto V  = glm::normalize(-ray.c.dir); // ray incoming direction
  const auto Vx = glm::normalize(-ray.x.dir); // diff ray incoming direction
  const auto Vy = glm::normalize(-ray.y.dir); // diff ray incoming direction
  const auto p  = hInfo.c.p;                  // surface position in world coordinate
  const auto px = ray.x.p + ray.x.dir * hInfo.x.z;
  const auto py = ray.y.p + ray.y.dir * hInfo.y.z;

  // coordinate
  const auto X = glm::normalize(glm::cross(glm::cross(N,V),N)); // Vx
  const auto Y = glm::normalize(N * glm::dot(N, V));            // Vy

  // index of refraction
  const float n1 = hInfo.c.hasFrontHit ? 1.f : ior;
  const float n2 = hInfo.c.hasFrontHit ? ior : 1.f;
  
  // refrection and reflection
  float cosI, sinI, cosIx, sinIx, cosIy, sinIy;
  float cosO, sinO, cosOx, sinOx, cosOy, sinOy;
  Point3 tDir, rDir, txDir, rxDir, tyDir, ryDir;
  do {
    // jitter normal
    const auto tjN = glm::normalize(N + rng->GetCirclePoint(refractionGlossiness));
    const auto rjN = glm::normalize(N + rng->GetCirclePoint(reflectionGlossiness));
    
    // incidence angle
    cosI  = glm::dot(tjN,V);
    sinI  = SQRT(1 - cosI * cosI);
    cosIx = glm::dot(tjN,Vx);
    sinIx = SQRT(1 - cosIx * cosIx);
    cosIy = glm::dot(tjN,Vy);
    sinIy = SQRT(1 - cosIy * cosIy);

    // refraction angle
    sinO  = MAX(0.f, MIN(1.f, sinI  * n1 / n2));
    cosO  = SQRT(1.f - sinO * sinO);  
    sinOx = MAX(0.f, MIN(1.f, sinIx * n1 / n2));
    cosOx = SQRT(1.f - sinOx * sinOx);
    sinOy = MAX(0.f, MIN(1.f, sinIy * n1 / n2));
    cosOy = SQRT(1.f - sinOy * sinOy);

    // ray directions
    tDir  = -X * sinO  - Y * cosO;
    txDir = -X * sinOx - Y * cosOx;
    tyDir = -X * sinOy - Y * cosOy;
    rDir  = 2.f*rjN*(glm::dot(rjN,V ))-V;
    rxDir = 2.f*rjN*(glm::dot(rjN,Vx))-Vx;
    ryDir = 2.f*rjN*(glm::dot(rjN,Vy))-Vy;
  }
  while ((glm::dot(tDir, Y) >  0.001f) ||
	 (glm::dot(txDir,Y) >  0.001f) ||
	 (glm::dot(tyDir,Y) >  0.001f) ||
	 (glm::dot(rDir, Y) < -0.001f) ||
	 (glm::dot(rxDir,Y) < -0.001f) ||
	 (glm::dot(ryDir,Y) < -0.001f));
    
  // reflection and transmission coefficients  
  const float C0 = (n1 - n2) * (n1 - n2) / (n1 + n2) / (n1 + n2);
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  assert(rC <= 1.f); assert(tC <= 1.f);

  // reflection and transmission colors
  const bool totReflection = (n1 * sinI / n2) > 1.001f;
  const Color sampleRefraction =
    hInfo.c.hasTexture ?
    refraction.Sample(hInfo.c.uvw, hInfo.c.duvw) : refraction.GetColor();
  const Color sampleReflection =
    hInfo.c.hasTexture ?
    reflection.Sample(hInfo.c.uvw, hInfo.c.duvw) : reflection.GetColor();
  const auto tK = totReflection ? Color(0.f) : sampleRefraction * tC;
  const auto rK = totReflection ?
    (sampleReflection + sampleRefraction) :
    (sampleReflection + sampleRefraction * rC);

  const float threshold = 0.001f;
  //!--- refraction ---
  if (bounceCount > 0 && 
      (tK.x > threshold || tK.y > threshold || tK.z > threshold)) 
  {
    DiffRay tRay(p, tDir, px, txDir, py, tyDir);
    DiffHitInfo tHit;
    tHit.c.z = BIGFLOAT;
    tRay.Normalize();
    if (TraceNodeNormal(rootNode, tRay, tHit)) {
      const auto K = tK * (tHit.c.hasFrontHit ?
                           Color(1.f) :
                           Attenuation(absorption, tHit.c.z));
      const auto* tMtl = tHit.c.node->GetMaterial();
      color += K * tMtl->Shade(tRay, tHit, lights, bounceCount - 1);
    }
    else {
      color += tK * environment.SampleEnvironment(tRay.c.dir);
    }
  }

  //!--- reflection ---
  if (bounceCount > 0 && 
      (rK.x > threshold || rK.y > threshold || rK.z > threshold))  
  {
    DiffRay rRay(p, rDir, px, rxDir, py, ryDir);
    DiffHitInfo rHit; 
    rRay.Normalize();
    rHit.c.z = BIGFLOAT;
    if (TraceNodeNormal(rootNode, rRay, rHit)) {
      const auto K = rK * (rHit.c.hasFrontHit ?
                           Color(1.f) :
                           Attenuation(absorption, rHit.c.z));
      const auto* rMtl = rHit.c.node->GetMaterial();
      color += K * rMtl->Shade(rRay, rHit, lights, bounceCount - 1);
    }
    else {
      color += rK * environment.SampleEnvironment(rRay.c.dir);
    }
  }

  //!--- normal shading ---
  const Color sampleDiffuse  =
    hInfo.c.hasTexture ?
    diffuse.Sample(hInfo.c.uvw, hInfo.c.duvw) :
    diffuse.GetColor();
  const Color sampleSpecular =
    hInfo.c.hasTexture ?
    specular.Sample(hInfo.c.uvw, hInfo.c.duvw) :
    specular.GetColor();
  if (hInfo.c.hasFrontHit) {
    for (auto& light : lights) {
      auto Intensity = light->Illuminate(p, N);
      if (light->IsAmbient()) {
        color += sampleDiffuse * Intensity;
      }
      else {
        auto L = glm::normalize(-light->Direction(p));
        auto H = glm::normalize(V + L);
        auto cosNL = MAX(0.f, glm::dot(N,L));
        auto cosNH = MAX(0.f, glm::dot(N,H));
        color += sampleDiffuse  * Intensity * cosNL;
        color += sampleSpecular * Intensity * POW(cosNH , glossiness) * cosNL;
      }
    }
  }

  //!--- process color ---
  color.r = MAX(0.f, MIN(1.f, color.r));
  color.g = MAX(0.f, MIN(1.f, color.g));
  color.b = MAX(0.f, MIN(1.f, color.b));
  return color;
}

//------------------------------------------------------------------------------

Color MtlPhong::Shade(const DiffRay &ray,
		      const DiffHitInfo &hInfo,
		      const LightList &lights,
		      int bounceCount)
  const
{
  Color color(0.f,0.f,0.f);
  auto p = hInfo.c.p;                  // surface position in world coordinate
  auto N = glm::normalize(hInfo.c.N);  // surface normal in world coordinate
  auto V = glm::normalize(-ray.c.dir); // ray incoming direction
  for (auto& light : lights) {
    auto I = light->Illuminate(p, N);
    if (light->IsAmbient()) {
      color += diffuse * light->Illuminate(p, N);
    }
    else {
      auto L = glm::normalize(-light->Direction(p));
      auto H = 2.f * glm::dot(L,N) * N - glm::normalize(L);
      auto cosNL = MAX(0.f, glm::dot(N,L));
      auto cosVH = MAX(0.f, glm::dot(V,H));
      color += diffuse * I * cosNL;
      color += specular * I * POW(cosVH, glossiness) * cosNL;
    }
  }
  color.r = MAX(0.f, MIN(1.f, color.r));
  color.g = MAX(0.f, MIN(1.f, color.g));
  color.b = MAX(0.f, MIN(1.f, color.b));
  return color;
}
