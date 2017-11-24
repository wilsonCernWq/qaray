#pragma once

#include <atomic>
#include "math/math.h"
#include "fb/tile.h"

//-----------------------------------------------------------------------------

class RenderImage {
 private:
  uchar *mask;
  Color24 *img;
  float *zbuffer;
  uchar *zbufferImg;
  uchar *sampleCount;
  uchar *sampleCountImg;
  uchar *irradComp;
  int width, height;
  std::atomic<int> numRenderedPixels;
 public:
  RenderImage();

  ~RenderImage();

  void Init(int w, int h);

  void AllocateIrradianceComputationImage();

  int GetWidth() const { return width; }

  int GetHeight() const { return height; }

  Color24 *GetPixels() { return img; }

  uchar *GetMasks() { return mask; }

  float *GetZBuffer() { return zbuffer; }

  uchar *GetZBufferImage() { return zbufferImg; }

  uchar *GetSampleCount() { return sampleCount; }

  uchar *GetSampleCountImage() { return sampleCountImg; }

  uchar *GetIrradianceComputationImage() { return irradComp; }

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
  bool SavePNG(const char *filename, uchar *data, int compCount) const;
};

//-----------------------------------------------------------------------------
