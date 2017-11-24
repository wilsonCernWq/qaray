//
// Created by qwu on 11/23/17.
//

#ifndef QARAY_SAMPLER_MT19937_H
#define QARAY_SAMPLER_MT19937_H
#pragma once

#include <random>
#include "core/qaray.h"
#include "core/sampler.h"
#include "math/math.h"

namespace qaray {
class Sampler_mt19937 : public Sampler {
 private:
  std::mt19937 rng;
  std::uniform_real_distribution<qaFLOAT> dist; // distribution in range [0, 1]
 public:
  Sampler_mt19937();
  void Get1f(qaFLOAT &r1) override;
  void Get2f(qaFLOAT &r1, qaFLOAT &r2) override;
  void Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3) override;
};
}
#endif //QARAY_SAMPLER_MT19937_H
