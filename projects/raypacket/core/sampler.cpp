///--------------------------------------------------------------------------//
///                                                                          //
/// Copyright(c) 2017-2018, Qi WU (University of Utah)                       //
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

#include "sampler.h"

namespace qaray {

qaFLOAT Halton(qaINT index, qaINT base) {
  qaFLOAT r = 0;
  qaFLOAT f = 1.0f/(qaFLOAT) base;
  for (qaINT i = index; i > 0; i /= base) {
    r += f*(i%base);
    f /= (qaFLOAT) base;
  }
  return r;
}

Point3 Sampler::UniformBall(qaFLOAT radius) {
  Point3 p;
  do {
    qaFLOAT r1(0), r2(0), r3(0);
    Get3f(r1, r2, r3);
    p.x = (2.f*r1 - 1.f)*radius;
    p.y = (2.f*r2 - 1.f)*radius;
    p.z = (2.f*r2 - 1.f)*radius;
  } while (length(p) > radius);
  return p;
}

Point3 Sampler::UniformHemisphere() {
  //
  // Generate random direction on unit hemisphere proportional to solid angle
  // PDF = 1 / 2PI
  //
  float r1(0), r2(0);
  Get2f(r1, r2);
  const qaFLOAT cosTheta = r1;
  const qaFLOAT sinTheta = sqrt(1 - r1*r1);
  const qaFLOAT phi = 2*PI*r2;
  const qaFLOAT x = sinTheta*cos(phi);
  const qaFLOAT y = sinTheta*sin(phi);
  return Point3(x, y, cosTheta);
}

Point3 Sampler::CosWeightedHemisphere() {
  //
  // Generate random direction on unit hemisphere proportional to
  // cosine-weighted solid angle
  //
  // PDF = cos(Theta) / PI
  //
  float r1(0), r2(0);
  Get2f(r1, r2);
  const qaFLOAT cosTheta = sqrt(r1);
  const qaFLOAT sinTheta = sqrt(1 - r1);
  const qaFLOAT phi = 2*PI*r2;
  const qaFLOAT x = sinTheta*cos(phi);
  const qaFLOAT y = sinTheta*sin(phi);
  return Point3(x, y, cosTheta);
}

qaFLOAT CosLobeWeightedFn(const qaINT n) {
  if (n==0) { return PI; }
  if (n==1) { return 2; }
  return static_cast<qaFLOAT>(n - 1)*CosLobeWeightedFn(n - 2)/n;
}

qaFLOAT Sampler::CosLobeWeightedHemisphereNormalization(qaINT N,
                                                        const Point3 &axis,
                                                        const Point3 &normal)
{
  const float cosTheta = glm::dot(axis, normal);
  const float sinTheta = sqrt(1.f - cosTheta*cosTheta);
  const float Theta = std::acos(cosTheta);
  float sum = 0.f;
  for (int i = 0; i < N - 1; i += 2) {
    sum += CosLobeWeightedFn(i)*pow(sinTheta, static_cast<float>(i));
  }
  sum *= cosTheta;
  if ((N%2)==0) // even
  {
    sum += 2.f*((float) M_PI - Theta);
  } else // odd
  {
    sum += (float) M_PI;
  }
  sum /= (float) (N + 1);
  return sum;
}

Point3 Sampler::CosLobeWeightedHemisphere(qaINT N, qaINT theta_max) {
  //
  // Generate random direction on unit hemisphere proportional to cosine lobe
  // around normal
  //
  float r1(0), r2(0);
  Get2f(r1, r2);
  if (theta_max==90) {
    //
    // PDF = (N+1) * (cos(theta) ^ N) / 2PI
    //
    const float cosTheta = pow(r1, 1.f/(N + 1));
    const float sinTheta = sqrt(1 - cosTheta*cosTheta);
    const float phi = 2*PI*r2;
    const float x = sinTheta*cos(phi);
    const float y = sinTheta*sin(phi);
    return Point3(x, y, cosTheta);
  } else {
    //
    // PDF = (N+1) * (cos(theta) ^ N) / 2PI * (theta_max / 90)
    //
    const float cosTheta =
        pow(1.f - r1*(1.f - pow(cos(theta_max), N + 1)), 1.f/(N + 1));
    const float sinTheta = sqrt(1 - cosTheta*cosTheta);
    const float phi = 2*PI*r2;
    const float x = sinTheta*cos(phi);
    const float y = sinTheta*sin(phi);
    return Point3(x, y, cosTheta);
  }
}
}

