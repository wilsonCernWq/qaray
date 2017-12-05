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
///--------------------------------------------------------------------------//
#include "math/math.h"
#include "scene/scene.h"
///--------------------------------------------------------------------------//
#include "tasking/parallel_for.h"
///--------------------------------------------------------------------------//

namespace qaray {
///--------------------------------------------------------------------------//
enum TimeState {START_FRAME, STOP_FRAME, KILL_FRAME};
void TimeFrame(TimeState state);
///--------------------------------------------------------------------------//
struct RendererParam {
  bool useSRGB = true;
  size_t sppMax = 16;
  size_t sppMin = 4;
  size_t photonMapSize = 1000;
  size_t photonMapBounce = 10;
  void SetPhotonMapSize(int size)
  {
    photonMapSize = static_cast<size_t>(size);
  }
  void SetSPPMax(int spp){ sppMax = static_cast<size_t>(spp); }
  void SetSPPMin(int spp){ sppMin = static_cast<size_t>(spp); }
  void SetSRGB(bool flag) { useSRGB = flag; }
};
class Renderer {
 protected:
  //!
  RendererParam& param;
  //!
  Scene* scene = nullptr;
  RenderImage* renderImage = nullptr;
  //! useful buffers
  Color3c *colorBuffer; // RGB qaUCHAR
  float   *depthBuffer;
  qaUCHAR *sampleCountBuffer;
  qaUCHAR *irradianceCountBuffer;
  qaUCHAR *maskBuffer;
  //!
  size_t pixelW, pixelH;       // global size in pixel
  size_t pixelRegion[4] = {0}; // local image offset [x y]
  size_t pixelSize[2] = {0};   // local image size
  //! camera
  float focal = 10.0f, dof = 0.0f;
  float screenW, screenH, aspect;
  Point3 screenX, screenY, screenZ;
  Point3 screenU, screenV, screenA;
  //! multi-threading parameters
  const size_t tileSize = 32; // this value should be platform dependent
  size_t tileDimX = 0;
  size_t tileDimY = 0;
  size_t tileCount = 0;
  //!
  size_t threadSize = 1;
  std::atomic<bool> threadStop;
  //! parameters used for MPI
  size_t mpiSize = 1;
  size_t mpiRank = 0;
 public:
  explicit Renderer(RendererParam &param);
  void ComputeScene(RenderImage& renderImage, Scene& scene);
  void ThreadRender();
  void PixelRender(size_t i, size_t j, size_t tile_idx);
  virtual void StartTimer() { TimeFrame(START_FRAME); }
  virtual void StopTimer() { TimeFrame(STOP_FRAME); }
  virtual void Init() {}
  virtual void Terminate() {};
  virtual void Render() = 0;
};
}

#endif //QARAY_RENDERER_H
