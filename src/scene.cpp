//------------------------------------------------------------------------------
///
/// \file       objects.cpp 
/// \author     Qi WU
/// \version    1.0
/// \date       August, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "scene.h"
#include <random>
#include <thread>
//------------------------------------------------------------------------------

//TBBHalton TBBHaltonRNG(HaltonRandom(0));
//
//HaltonRandom::HaltonRandom(int seed)
//{
//  idx = seed;
//};
//void HaltonRandom::Seed(int seed)
//{
//  idx = seed;
//}
//void HaltonRandom::Get(float &r1, float &r2)
//{
//  r1 = Halton(idx, 2);
//  r2 = Halton(idx, 3);
//  Increment();
//}
//void HaltonRandom::Increment()
//{
//  idx = idx + 1;
//}

//------------------------------------------------------------------------------
//
//// rendering example_project9.xml takes 23.224073 s
//struct UniformRandom_mt19937 : public UniformRandom {
//  std::mt19937 rng;
//  std::uniform_real_distribution<float> dist; // distribution in range [0, 1]
//  UniformRandom_mt19937()
//  {
//    rng.seed(std::random_device()());
//    dist = std::uniform_real_distribution<float>(0.0, 1.0);
//  }
//
//  float Get() { return dist(rng); }
//};
//
//// reference https://en.wikipedia.org/wiki/Xorshift
//// rendering example_project9.xml takes 20.582386 s
//struct UniformRandom_Marsaglia : public UniformRandom {
//  static uint32_t seed;
//
//  /* The state word must be initialized to non-zero */
//  uint32_t xorshift32()
//  {
//    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
//    uint32_t x = seed;
//    x ^= x << 13;
//    x ^= x >> 17;
//    x ^= x << 5;
//    seed = x;
//    return x;
//  }
//
//  float Get()
//  {
//    return xorshift32() / (float) (POW(2, 32) - 1);
//  }
//};
//
//uint32_t UniformRandom_Marsaglia::seed = 123456789;

//------------------------------------------------------------------------------

ThreadSampler *rng = new ThreadSampler(Sampler_Marsaglia());

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Trace the ray within this node and all its children
//------------------------------------------------------------------------------

bool TraceNodeShadow
    (Node &node, Ray &ray, HitInfo &hInfo)
{
  Ray nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()
        ->IntersectRay(nodeRay, hInfo, HIT_FRONT_AND_BACK)) { return true; }
  }
  for (size_t c = 0; c < node.GetNumChild(); ++c) {
    if (TraceNodeShadow(*(node.GetChild(c)), nodeRay, hInfo)) { return true; }
  }
  return false;
}

//------------------------------------------------------------------------------

bool TraceNodeNormal
    (Node &node, DiffRay &ray, DiffHitInfo &hInfo /* it stores results */)
{
  bool hasHit = false;
  // We first check if this ray will intersect with the object held by
  // this node
  DiffRay nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()->IntersectRay(nodeRay.c, hInfo.c, HIT_FRONT_AND_BACK,
                                        &nodeRay, &hInfo)) {
      hInfo.c.node = &node;
      hasHit = true;
    }
  }
  // Now we still need to check all the childs contained by this node
  for (int c = 0; c < node.GetNumChild(); ++c) {
    // Reaches here means this node has at lease one child
    if (TraceNodeNormal(*(node.GetChild(c)), nodeRay, hInfo)) { hasHit = true; }
  }
  if (hasHit) { node.FromNodeCoords(hInfo); }
  return hasHit;
}

//------------------------------------------------------------------------------

SuperSamplerHalton::SuperSamplerHalton(const Color3f th,
                                       const int sppMin,
                                       const int sppMax)
    : th(th), sppMin(sppMin), sppMax(sppMax) {}

const Color3f &SuperSamplerHalton::GetColor() const { return color; }

int SuperSamplerHalton::GetSampleID() const { return s; }

bool SuperSamplerHalton::Loop() const
{
  return (s < sppMin ||
      (s >= sppMin && s < sppMax &&
          (color_std.r > th.r || color_std.g > th.g || color_std.b > th.b)));
}

Point3 SuperSamplerHalton::NewPixelSample()
{
  return Point3(Halton(s, 11), Halton(s, 13), 0.f);
}

Point3 SuperSamplerHalton::NewDofSample(const float R)
{
  float r1, r2;
  rng->local().Get2f(r1, r2);
  const float r = R * SQRT(r1);
  const float t = r2 * 2.f * PI;
  return Point3(r * COS(t), r * SIN(t), 0.f);
}

void SuperSamplerHalton::Accumulate(const Color3f &localColor)
{
  const Color3f dc = (localColor - color) / static_cast<float>(s + 1);
  color += dc;
  color_std += s > 0 ?
               dc * dc * static_cast<float>(s + 1)
                   - color_std / static_cast<float>(s) :
               Color3f(0.0f);
}

void SuperSamplerHalton::Increment() { ++s; }

//------------------------------------------------------------------------------
