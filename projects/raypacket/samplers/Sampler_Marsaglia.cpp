//
// Created by qwu on 11/23/17.
//

#include <ctime>
#include "Sampler_Marsaglia.h"

namespace qaray {
qaFLOAT Sampler_Marsaglia::xorshift32()
{
  if (!initialized) {
    seed = static_cast<uint32_t >(time(nullptr));
    initialized = true;
  }
  /* The seed word must be initialized to non-zero */
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  auto x = static_cast<uint32_t>(seed);
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  seed = x;
  return x / (qaFLOAT) (pow(2, 32) - 1);
}
void Sampler_Marsaglia::Get1f(qaFLOAT &r1)
{
  r1 = xorshift32();
}
void Sampler_Marsaglia::Get2f(qaFLOAT &r1, qaFLOAT &r2)
{
  r1 = xorshift32();
  r2 = xorshift32();
}
void Sampler_Marsaglia::Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3)
{
  r1 = xorshift32();
  r2 = xorshift32();
  r3 = xorshift32();
}
}