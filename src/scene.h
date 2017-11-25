//-----------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 6, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-----------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

//-----------------------------------------------------------------------------

#include <iostream>
#include <string>
#include <vector>
#include <atomic>

#include "core/core.h"
#include "math/math.h"

#ifdef USE_TBB
# include <tbb/task_arena.h>
# include <tbb/task_scheduler_init.h>
# include <tbb/parallel_for.h>
# include <tbb/enumerable_thread_specific.h>
#endif

#ifdef USE_OMP
# include <omp.h>
#endif

//-----------------------------------------------------------------------------


//------------------------------------------------------------------------------

/* inline float Halton (int index, int base) */
/* { */
/*   float r = 0; */
/*   float f = 1.0f / (float) base; */
/*   for (int i = index; i > 0; i /= base) */
/*   { */
/*     r += f * (i % base); */
/*     f /= (float) base; */
/*   } */
/*   return r; */
/* } */
//
//struct HaltonRandom {
//  int idx = 0;
//  HaltonRandom(int);
//  void Seed(int seed);
//  void Get(float &r1, float &r2);
//  void Increment();
//};
//
//typedef tbb::enumerable_thread_specific<HaltonRandom> TBBHalton;
//extern TBBHalton TBBHaltonRNG;
//
//struct UniformRandom {
//  virtual float Get() = 0;
//};

typedef tbb::enumerable_thread_specific<Sampler*> TBBSampler;
extern TBBSampler rng;

//-----------------------------------------------------------------------------

bool TraceNodeShadow(Node &node, Ray &ray, HitInfo &hInfo);

bool TraceNodeNormal(Node &node, DiffRay &ray, DiffHitInfo &hInfo);

//-----------------------------------------------------------------------------

class SuperSampler {
 public:
  virtual const Color3f &GetColor() const = 0;

  virtual int GetSampleID() const = 0;

  virtual bool Loop() const = 0;

  virtual Point3 NewPixelSample() = 0;

  virtual Point3 NewDofSample(const float) = 0;

  virtual void Accumulate(const Color3f &localColor) = 0;

  virtual void Increment() = 0;
};

class SuperSamplerHalton : public SuperSampler {
 private:
  const Color3f th;
  const int sppMin, sppMax;
  Color3f color_std = Color3f(0.0f, 0.0f, 0.0f);
  Color3f color = Color3f(0.0f, 0.0f, 0.0f);
  int s = 0;
 public:
  SuperSamplerHalton(const Color3f th, const int sppMin, const int sppMax);

  const Color3f &GetColor() const;

  int GetSampleID() const;

  bool Loop() const;

  Point3 NewPixelSample();

  Point3 NewDofSample(const float);

  void Accumulate(const Color3f &localColor);

  void Increment();
};
//-----------------------------------------------------------------------------

#endif
