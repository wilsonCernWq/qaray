#pragma once

#include <atomic>
#include "math/math.h"
#include "fb/tile.h"

//-----------------------------------------------------------------------------

class RenderImage {
 private:
  uint8_t *mask;
  Color3c *img;
  float *zbuffer;
  uint8_t *zbufferImg;
  uint8_t *sampleCount;
  uint8_t *sampleCountImg;
  uint8_t *irradComp;
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

  uint8_t *GetMasks() { return mask; }

  float *GetZBuffer() { return zbuffer; }

  uint8_t *GetZBufferImage() { return zbufferImg; }

  uint8_t *GetSampleCount() { return sampleCount; }

  uint8_t *GetSampleCountImage() { return sampleCountImg; }

  uint8_t *GetIrradianceComputationImage() { return irradComp; }

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
  bool SavePNG(const char *filename, uint8_t *data, int compCount) const;
};

//-----------------------------------------------------------------------------
