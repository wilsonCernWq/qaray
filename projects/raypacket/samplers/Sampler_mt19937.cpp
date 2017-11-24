//
// Created by qwu on 11/23/17.
//

#include "Sampler_mt19937.h"

namespace qaray {
Sampler_mt19937::Sampler_mt19937()
{
  rng.seed(std::random_device()());
  dist = std::uniform_real_distribution<qaFLOAT>(0.0, 1.0);
}
void Sampler_mt19937::Get1f(qaFLOAT &r1)
{
  r1 = dist(rng);
}
void Sampler_mt19937::Get2f(qaFLOAT &r1, qaFLOAT &r2)
{
  r1 = dist(rng);
  r2 = dist(rng);
}
void Sampler_mt19937::Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3)
{
  r1 = dist(rng);
  r2 = dist(rng);
  r3 = dist(rng);
}
}