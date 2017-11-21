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

//------------------------------------------------------------------------------

std::vector<HaltonRandom> haltonRNG;

//------------------------------------------------------------------------------

// rendering example_project9.xml takes 23.224073 s
struct UniformRandom_mt19937 : public UniformRandom
{
  std::mt19937 rng;
  std::uniform_real_distribution<float> dist; // distribution in range [0, 1]
  UniformRandom_mt19937 ()
  {
    rng.seed(std::random_device()());
    dist = std::uniform_real_distribution<float>(0.0, 1.0);
  }

  float Get () { return dist(rng); }
};

// reference https://en.wikipedia.org/wiki/Xorshift
// rendering example_project9.xml takes 20.582386 s
struct UniformRandom_Marsaglia : public UniformRandom
{
  static uint32_t seed;

  /* The state word must be initialized to non-zero */
  uint32_t xorshift32 ()
  {
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    uint32_t x = seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    seed = x;
    return x;
  }

  float Get ()
  {
    return xorshift32() / (float) (POW(2, 32) - 1);
  }
};

uint32_t UniformRandom_Marsaglia::seed = 123456789;

//------------------------------------------------------------------------------

UniformRandom *rng = new UniformRandom_Marsaglia;

//------------------------------------------------------------------------------

Point3 GetCirclePoint (const float r1, const float r2, const float r3, float size)
{
  Point3 p;
  do
  {
    p.x = (2.f * r1 - 1.f) * size;
    p.y = (2.f * r2 - 1.f) * size;
    p.z = (2.f * r3 - 1.f) * size;
  } while (glm::length(p) > size);
  return p;
}

Point3 UniformSampleHemiSphere (const float r1, const float r2)
{
  // Generate random direction on unit hemisphere proportional to solid angle
  // PDF = 1 / 2PI 
  const float cosTheta = r1;
  const float sinTheta = SQRT(1 - r1 * r1);
  const float phi = 2 * M_PI * r2;
  const float x = sinTheta * COS(phi);
  const float y = sinTheta * SIN(phi);
  return Point3(x, y, cosTheta);
}

Point3 CosWeightedSampleHemiSphere (const float r1, const float r2)
{
  // Generate random direction on unit hemisphere proportional to cosine-weighted solid angle
  // PDF = cos(Theta) / PI
  const float cosTheta = SQRT(r1);
  const float sinTheta = SQRT(1 - r1);
  const float phi = 2 * M_PI * r2;
  const float x = sinTheta * COS(phi);
  const float y = sinTheta * SIN(phi);
  return Point3(x, y, cosTheta);
}

float CosLobeWeightedFn (const int n)
{
  if (n == 0) { return M_PI; }
  if (n == 1) { return 2; }
  return static_cast<float>(n-1)*CosLobeWeightedFn(n-2)/n;
}

float CosLobeWeightedNormalization (const int n, const Point3 axis, const Point3 norm)
{
  const float cosTheta = glm::dot(axis,norm);
  const float sinTheta = SQRT(1.f - cosTheta * cosTheta);
  const float Theta = std::acos(cosTheta);
  float sum = 0.f;
  for (int i = 0; i < n-1; i+= 2)
  {
    sum += CosLobeWeightedFn(i) * POW(sinTheta, i);
  }
  sum *= cosTheta;
  if ((n%2) == 0) // even
  {
    sum += 2.f * ((float)M_PI - Theta);
  }
  else // odd
  {
    sum += (float)M_PI;
  }
  sum /= (float) (n+1);
  return sum;
}

Point3 CosLobeWeightedSampleHemiSphere (const float r1, const float r2,
					const int N, const int theta_max)
{
  // Generate random direction on unit hemisphere proportional to cosine lobe around normal
  if (theta_max == 90)
  {
    // PDF = (N+1) * (cos(theta) ^ N) / 2PI
    const float cosTheta = POW(r1, 1.f / (N + 1));
    const float sinTheta = SQRT(1 - cosTheta * cosTheta);
    const float phi = 2 * M_PI * r2;
    const float x = sinTheta * COS(phi);
    const float y = sinTheta * SIN(phi);
    return Point3(x, y, cosTheta);
  } else
  {
    // PDF = (N+1) * (cos(theta) ^ N) / 2PI * (theta_max / 90)
    const float cosTheta = POW(1.f - r1 * (1.f - POW(COS(theta_max), N + 1)), 1.f / (N + 1));
    const float sinTheta = SQRT(1 - cosTheta * cosTheta);
    const float phi = 2 * M_PI * r2;
    const float x = sinTheta * COS(phi);
    const float y = sinTheta * SIN(phi);
    return Point3(x, y, cosTheta);
  }
}

//------------------------------------------------------------------------------

const float DiffRay::dx = 0.01f;
const float DiffRay::dy = 0.01f;
const float DiffRay::rdx = 1.f / DiffRay::dx;
const float DiffRay::rdy = 1.f / DiffRay::dy;

//------------------------------------------------------------------------------
// Trace the ray within this node and all its children
//------------------------------------------------------------------------------

bool TraceNodeShadow
    (Node &node, Ray &ray, HitInfo &hInfo)
{
  Ray nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL)
  {
    if (node.GetNodeObj()->IntersectRay(nodeRay, hInfo, HIT_FRONT_AND_BACK)) { return true; }
  }
  for (size_t c = 0; c < node.GetNumChild(); ++c)
  {
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
  if (node.GetNodeObj() != NULL)
  {
    if (node.GetNodeObj()->IntersectRay(nodeRay.c, hInfo.c, HIT_FRONT_AND_BACK,
                                        &nodeRay, &hInfo))
    {
      hInfo.c.node = &node;
      hasHit = true;
    }
  }
  // Now we still need to check all the childs contained by this node
  for (int c = 0; c < node.GetNumChild(); ++c)
  {
    // Reaches here means this node has at lease one child
    if (TraceNodeNormal(*(node.GetChild(c)), nodeRay, hInfo)) { hasHit = true; }
  }
  if (hasHit) { node.FromNodeCoords(hInfo); }
  return hasHit;
}

//------------------------------------------------------------------------------

SuperSamplerHalton::SuperSamplerHalton (const Color th, const int sppMin, const int sppMax)
    : th(th), sppMin(sppMin), sppMax(sppMax) {}

const Color &SuperSamplerHalton::GetColor () const { return color; }

const int SuperSamplerHalton::GetSampleID () const { return s; }

bool SuperSamplerHalton::Loop () const
{
  return (s < sppMin ||
          (s >= sppMin && s < sppMax &&
           (color_std.r > th.r || color_std.g > th.g || color_std.b > th.b)));
}

Point3 SuperSamplerHalton::NewPixelSample ()
{
  return Point3(Halton(s, 2), Halton(s, 3), 0.f);
}

Point3 SuperSamplerHalton::NewDofSample (const float R)
{
  const float r = R * SQRT(rng->Get());
  const float t = rng->Get() * 2.f * M_PI;
  return Point3(r * cos(t), r * sin(t), 0.f);
}

void SuperSamplerHalton::Accumulate (const Color &localColor)
{
  const Color dc = (localColor - color) / static_cast<float>(s + 1);
  color += dc;
  color_std += s > 0 ?
               dc * dc * static_cast<float>(s + 1) - color_std / static_cast<float>(s) :
               Color(0.0f);
}

void SuperSamplerHalton::Increment () { ++s; }

//------------------------------------------------------------------------------
