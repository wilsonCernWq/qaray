///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/23/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.             //
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
#pragma once

#include <atomic>
#include "math/math.h"
#include "fb/tile.h"

//-----------------------------------------------------------------------------

class RenderImage {
 private:
  qaUCHAR *mask;
  Color3c *img;
  float *zbuffer;
  qaUCHAR *zbufferImg;
  qaUCHAR *sampleCount;
  qaUCHAR *sampleCountImg;
  qaUCHAR *irradComp;
  int width, height;
  std::atomic<int> numRenderedPixels;
 public:
  RenderImage();

  ~RenderImage();

  void Init(int w, int h);

  void AllocateIrradianceComputationImage();

  int GetWidth() const { return width; }

  int GetHeight() const { return height; }

  Color3c *GetPixels() { return img; }

  qaUCHAR *GetMasks() { return mask; }

  float *GetZBuffer() { return zbuffer; }

  qaUCHAR *GetZBufferImage() { return zbufferImg; }

  qaUCHAR *GetSampleCount() { return sampleCount; }

  qaUCHAR *GetSampleCountImage() { return sampleCountImg; }

  qaUCHAR *GetIrradianceComputationImage() { return irradComp; }

  void ResetNumRenderedPixels();

  int GetNumRenderedPixels() const { return numRenderedPixels; }

  void IncrementNumRenderPixel(int n) { numRenderedPixels += n; }

  bool IsRenderDone() const { return numRenderedPixels >= width * height; }

  void ComputeZBufferImage();

  int ComputeSampleCountImage();

  bool SaveImage(const char *filename) const;

  bool SaveZImage(const char *filename) const;

  bool SaveSampleCountImage(const char *filename) const;

  bool SaveIrradianceComputationImage(const char *filename) const;

 private:
  bool SavePNG(const char *filename, qaUCHAR *data, int compCount) const;
};

//-----------------------------------------------------------------------------
