///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/3/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.              //
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

#ifndef QARAY_SCENE_H
#define QARAY_SCENE_H
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <atomic>

///--------------------------------------------------------------------------//
#include "math/math.h"
#include "core/core.h"
///--------------------------------------------------------------------------//
#include "fb/framebuffer.h"
///--------------------------------------------------------------------------//
#include "samplers/sampler_selection.h"
///--------------------------------------------------------------------------//

namespace qaray {
///--------------------------------------------------------------------------//
class Scene {
 public:
  Node rootNode;
  Camera camera;
  MaterialList materials;
  LightList lights;
  ObjFileList objList;
  TexturedColor background;
  TexturedColor environment;
  TextureList textureList;
 public:
  bool TraceNodeShadow(Node &node, Ray &ray, HitInfo &hInfo);
  bool TraceNodeNormal(Node &node, DiffRay &ray, DiffHitInfo &hInfo);
};
extern Scene scene;
///--------------------------------------------------------------------------//
extern RenderImage renderImage;
///--------------------------------------------------------------------------//

}
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

#endif//QARAY_SCENE_H
