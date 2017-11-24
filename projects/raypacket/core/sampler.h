//
// Created by qwu on 11/23/17.
//

#ifndef QARAY_SAMPLER_H
#define QARAY_SAMPLER_H

#include "math/math.h"

namespace qaray {

qaFLOAT Halton(qaINT index, qaINT base);

class Sampler {
 public:
  Sampler() = default;
  virtual ~Sampler() = default;
  virtual void Get1f(qaFLOAT &r) = 0;
  virtual void Get2f(qaFLOAT &,
                     qaFLOAT &) = 0;
  virtual void Get3f(qaFLOAT &,
                     qaFLOAT &,
                     qaFLOAT &) = 0;
  virtual Point3 UniformBall(qaFLOAT radius);
  virtual Point3 UniformHemisphere();
  virtual Point3 CosWeightedHemisphere();
  virtual Point3 CosLobeWeightedHemisphere(qaINT N,
                                           qaINT theta_max = 90);
  qaFLOAT CosLobeWeightedHemisphereNormalization(qaINT N,
                                                 const Point3& axis,
                                                 const Point3& normal);
};
}

#endif //QARAY_SAMPLER_H
