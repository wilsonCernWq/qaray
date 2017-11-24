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

#define TEXTURE_SAMPLE_COUNT 32


#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>

#include "core/sampler.h"
#include "core/camera.h"
#include "core/transform.h"
#include "core/box.h"
#include "core/ray.h"
#include "core/hitinfo.h"
#include "core/light.h"
#include "core/material.h"
#include "core/object.h"
#include "core/items.h"
#include "core/node.h"

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


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


//-------------------------------------------------------------------------------

class Texture : public ItemBase {
 public:
  // Evaluates the color at the given uvw location.
  virtual Color3f Sample(const Point3 &uvw) const =0;

  // Evaluates the color around the given uvw location using the derivatives duvw
  // by calling the Sample function multiple times.
  virtual Color3f Sample(const Point3 &uvw,
                       const Point3 duvw[2],
                       bool elliptic = true) const
  {
    Color3f c = Sample(uvw);
    if (glm::length2(duvw[0]) + glm::length2(duvw[1]) == 0) return c;
    for (int i = 1; i < TEXTURE_SAMPLE_COUNT; i++) {
      float x = Halton(i, 2);
      float y = Halton(i, 3);
      if (elliptic) {
        float r = sqrtf(x) * 0.5f;
        x = r * sinf(y * (float) M_PI * 2);
        y = r * cosf(y * (float) M_PI * 2);
      } else {
        if (x > 0.5f) x -= 1;
        if (y > 0.5f) y -= 1;
      }
      c += Sample(uvw + x * duvw[0] + y * duvw[1]);
    }
    return c / float(TEXTURE_SAMPLE_COUNT);
  }

  virtual bool SetViewportTexture() const { return false; }// used for OpenGL display
 protected:
  // Clamps the uvw values for tiling textures, such that all values fall between 0 and 1.
  static Point3 TileClamp(const Point3 &uvw)
  {
    Point3 u;
    u.x = uvw.x - (int) uvw.x;
    u.y = uvw.y - (int) uvw.y;
    u.z = uvw.z - (int) uvw.z;
    if (u.x < 0) u.x += 1;
    if (u.y < 0) u.y += 1;
    if (u.z < 0) u.z += 1;
    return u;
  }
};

typedef ItemFileList<Texture> TextureList;

//-------------------------------------------------------------------------------

// This class handles textures with texture transformations.
// The uvw values passed to the Sample methods are transformed
// using the texture transformation.
class TextureMap : public Transformation {
 public:
  TextureMap() : texture(NULL) {}

  TextureMap(Texture *tex) : texture(tex) {}

  void SetTexture(Texture *tex) { texture = tex; }

  virtual Color3f Sample(const Point3 &uvw) const
  {
    return texture ? texture->Sample(TransformTo(uvw)) : Color3f(0, 0, 0);
  }

  virtual Color3f Sample(const Point3 &uvw,
                       const Point3 duvw[2],
                       bool elliptic = true) const
  {
    if (texture == NULL) return Color3f(0, 0, 0);
    Point3 u = TransformTo(uvw);
    Point3 d[2];
    d[0] = TransformTo(duvw[0] + uvw) - u;
    d[1] = TransformTo(duvw[1] + uvw) - u;
    return texture->Sample(u, d, elliptic);
  }

  bool SetViewportTexture() const
  {
    if (texture) return texture->SetViewportTexture();
    return false;
  }// used for OpenGL display
 private:
  Texture *texture;
};

//-------------------------------------------------------------------------------

// This class keeps a TextureMap and a color. This is useful for keeping material
// color parameters that can also be textures. If no texture is specified, it
// automatically uses the color value. Otherwise, the texture value is multiplied
// by the color value.
class TexturedColor {
 private:
  Color3f color;
  TextureMap *map;
 public:
  TexturedColor() : color(0, 0, 0), map(NULL) {}

  TexturedColor(float r, float g, float b) : color(r, g, b), map(NULL) {}

  virtual ~TexturedColor() { if (map) delete map; }

  void SetColor(const Color3f &c) { color = c; }

  void SetTexture(TextureMap *m)
  {
    if (map) delete map;
    map = m;
  }

  Color3f GetColor() const { return color; }

  const TextureMap *GetTexture() const { return map; }

  Color3f Sample(const Point3 &uvw) const
  {
    return (map) ? color * map->Sample(uvw) : color;
  }

  Color3f Sample(const Point3 &uvw,
               const Point3 duvw[2],
               bool elliptic = true) const
  {
    return (map) ? color * map->Sample(uvw, duvw, elliptic) : color;
  }

  // Returns the color value at the given direction for environment mapping.
  Color3f SampleEnvironment(const Point3 &dir) const
  {
    float z = asinf(-dir.z) / float(M_PI) + 0.5f;
    float x = dir.x / (fabs(dir.x) + fabs(dir.y));
    float y = dir.y / (fabs(dir.x) + fabs(dir.y));
    return Sample(Point3(0.5f, 0.5f, 0.0f) + z
        * (x * Point3(0.5f, 0.5f, 0) + y * Point3(-0.5f, 0.5f, 0)));
  }
};

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------


#endif
