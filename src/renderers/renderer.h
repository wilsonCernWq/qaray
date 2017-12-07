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

#ifndef QARAY_RENDERER_H
#define QARAY_RENDERER_H
#pragma once

///--------------------------------------------------------------------------//
#include <cstdlib>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <functional>
///--------------------------------------------------------------------------//
#include "math/math.h"
#include "scene/scene.h"
///--------------------------------------------------------------------------//
#include "tasking/parallel_for.h"
///--------------------------------------------------------------------------//

namespace qaray {
///--------------------------------------------------------------------------//
///--------------------------------------------------------------------------//
struct RendererParam {
  qaBOOL useSRGB = true;
  size_t sppMax = 8;
  size_t sppMin = 4;
  size_t photonMapSize = size_t(10000);
  size_t photonMapBounce = 5;
  qaFLOAT photonMapRadius = 1.f;
  size_t causticsMapSize = size_t(1000);
  size_t causticsMapBounce = 5;
  qaFLOAT causticsMapRadius = 1.f;
  void SetPhotonMapBounce(size_t b) { photonMapBounce = b; }
  void SetPhotonMapSize(size_t sz) { photonMapSize = sz; }
  void SetPhotonMapRadius(qaFLOAT r) { photonMapRadius = r; }
  void SetCausticsMapBounce(size_t b) { causticsMapBounce = b; }
  void SetCausticsMapSize(size_t sz) { causticsMapSize = sz; }
  void SetCausticsMapRadius(qaFLOAT r) { causticsMapRadius = r; }
  void SetSPPMax(int spp) { sppMax = static_cast<size_t>(spp); }
  void SetSPPMin(int spp) { sppMin = static_cast<size_t>(spp); }
  void SetSRGB(bool flag) { useSRGB = flag; }
};
class Renderer {
 protected:
  //! get all user defined parameters
  RendererParam &param;
  //! load scene
  Scene *scene = nullptr;
  FrameBuffer *image = nullptr;
  //! handy buffers
  Color3c *colorBuffer; // RGB qaUCHAR
  float *depthBuffer;
  qaUCHAR *sampleCountBuffer;
  qaUCHAR *irradianceCountBuffer;
  qaUCHAR *maskBuffer;
  //! canvas
  size_t pixelW, pixelH;       // global size in pixel
  size_t pixelRegion[4] = {0}; // local image offset [x y]
  size_t pixelSize[2] = {0};   // local image size
  //! camera
  float focal = 10.0f, dof = 0.0f;
  float screenW, screenH, aspect;
  Point3 screenX, screenY, screenZ;
  Point3 screenU, screenV, screenA;
  //! multi-threading information
  const size_t tileSize = 32; // this value should be platform dependent
  size_t tileDimX = 0;
  size_t tileDimY = 0;
  size_t tileCount = 0;
  //! MPI information
  size_t mpiSize = 1;
  size_t mpiRank = 0;
 public:
  explicit Renderer(RendererParam &param);
  void ComputeScene(FrameBuffer &renderImage, Scene &scene);
  void ThreadRender();
  void PixelRender(size_t i, size_t j, size_t tile_idx);
  virtual void StartTimer();
  virtual void StopTimer();
  virtual void KillTimer();
  virtual void Init();
  virtual void Terminate();
  virtual void Render() = 0;
};
}

#endif //QARAY_RENDERER_H
