//
// Created by qwu on 11/23/17.
//

#ifndef QARAY_SAMPLER_MARSAGLIA_H
#define QARAY_SAMPLER_MARSAGLIA_H
#pragma once

#include "core/qaray.h"
#include "core/sampler.h"
#include "math/math.h"

namespace qaray {
class Sampler_Marsaglia : public Sampler {
 private:
  bool initialized = false;
  uint32_t seed;
  qaFLOAT xorshift32(); /* type: uint32 */
 public:
  void Get1f(qaFLOAT &r1) override;
  void Get2f(qaFLOAT &r1, qaFLOAT &r2) override;
  void Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3) override;
};
}

#endif //QARAY_SAMPLER_MARSAGLIA_H

