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

extern Node rootNode;

Color Attenuation(const Color& absorption, const float l) {
  const float R = exp(-absorption.r * l);
  const float G = exp(-absorption.g * l);
  const float B = exp(-absorption.b * l);
  return Color(R, G, B); // attenuation
}

Color MtlBlinn::Shade
(const Ray &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const
{
  Color color(0.f,0.f,0.f);
  const auto p = hInfo.p;                  // surface position in world coordinate
  const auto N = glm::normalize(hInfo.N);  // surface normal in world coordinate
  const auto V = glm::normalize(-ray.dir); // ray incoming direction
  
  const float cosI = glm::dot(N,V);
  const float sinI = SQRT(1 - cosI * cosI);
  
  const auto X = glm::normalize(V - N * cosI); // Vx
  const auto Y = glm::normalize(N * cosI);     // Vy
  
  const float n1 = hInfo.front ? 1.f : ior;
  const float n2 = hInfo.front ? ior : 1.f;
  
  const float C0 = (n1 - n2) * (n1 - n2) / (n1 + n2) / (n1 + n2);
  const float rC = C0 + (1.f - C0) * POW(1.f - ABS(cosI), 5.f);
  const float tC = 1.f - rC;
  assert(rC <= 1.f); assert(tC <= 1.f);

  const bool totReflection = (n1 * sinI / n2) > 1.001f;
  const auto tK = totReflection ? Color(0.f) : refraction * tC;
  const auto rK = totReflection ? (reflection + refraction) : (reflection + refraction * rC);

  const float threshold = 0.001f;
  //!--- refraction ---
  if (bounceCount > 0 && (tK.x > threshold || tK.y > threshold || tK.z > threshold)) {
    const float sinO = MAX(0.f, MIN(1.f, sinI * n1 / n2));
    const float cosO = SQRT(1.f - sinO * sinO);
    Ray tRay(p, (-X * sinO - Y * cosO)); tRay.Normalize();
    HitInfo tHit; tHit.z = BIGFLOAT;
    if (TraceNodeNormal(rootNode, tRay, tHit)) {
      const auto K = tK * (tHit.front ? Color(1.f) : Attenuation(absorption, tHit.z));
      const auto* tMtl = tHit.node->GetMaterial();
      color += K * tMtl->Shade(tRay, tHit, lights, bounceCount - 1);
    }
  }
  
  //!--- reflection ---
  if (bounceCount > 0 && (rK.x > threshold || rK.y > threshold || rK.z > threshold)) {
    Ray rRay(p, 2.f * N * glm::dot(N,V) - V); rRay.Normalize();
    HitInfo rHit; rHit.z = BIGFLOAT;
    if (TraceNodeNormal(rootNode, rRay, rHit)) {
      const auto K = rK * (rHit.front ? Color(1.f) : Attenuation(absorption, rHit.z));
      const auto* rMtl = rHit.node->GetMaterial();
      color += K * rMtl->Shade(rRay, rHit, lights, bounceCount - 1);
    }
  }
  
  //!--- normal shading ---
  if (hInfo.front) {
    for (auto& light : lights) {
      auto Intensity = light->Illuminate(p, N);
      if (light->IsAmbient()) {
        color += diffuse * Intensity;
      }
      else {
        auto L = glm::normalize(-light->Direction(p));
        auto H = glm::normalize(V + L);
        auto cosNL = MAX(0.f, glm::dot(N,L));
        auto cosNH = MAX(0.f, glm::dot(N,H));
        color += diffuse * Intensity * cosNL;
        color += specular * Intensity * POW(cosNH , glossiness) * cosNL;
      }
    }
  }
  
  //!--- process color ---
  color.r = MAX(0.f, MIN(1.f, color.r));
  color.g = MAX(0.f, MIN(1.f, color.g));
  color.b = MAX(0.f, MIN(1.f, color.b));
  return color;
}

Color MtlPhong::Shade
(const Ray &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const
{
  Color color(0.f,0.f,0.f);
  auto p = hInfo.p;                  // surface position in world coordinate
  auto N = glm::normalize(hInfo.N);  // surface normal in world coordinate
  auto V = glm::normalize(-ray.dir); // ray incoming direction
  for (auto& light : lights) {
    auto I = light->Illuminate(p, N);
    if (light->IsAmbient()) {
      color += diffuse * light->Illuminate(p, N);
    }
    else {
      auto L = -glm::normalize(light->Direction(p));
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
