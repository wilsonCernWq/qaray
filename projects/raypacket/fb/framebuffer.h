//------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.1
/// \date       August 29, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///
/// \file       framebuffer.h 
/// \author     Qi WU
///
/// \brief Framebuffer class for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "fb/tile.h"
#include "math/math.h"
#include "lodepng.h"
#include <atomic>

//------------------------------------------------------------------------------

namespace qw {
  class RenderImage
  {
  private:
    Color3c *img;
    float   *zbuffer;
    uchar   *zbufferImg;
    int      width, height;
    std::atomic<int> numRenderedPixels;
  public:
    RenderImage();
    void Init(int w, int h);
    
    int GetWidth() const;
    int GetHeight() const;
    Color3c* GetPixels();
    float*   GetZBuffer();
    uchar*   GetZBufferImage();
    
    void ResetNumRenderedPixels();
    int	 GetNumRenderedPixels() const;
    void IncrementNumRenderPixel(int n);
    bool IsRenderDone() const;
    
    void ComputeZBufferImage();
    
    bool SaveImage (const char *filename) const;
    bool SaveZImage(const char *filename) const;
    
  private:
    bool SavePNG(const char *filename, uchar *data, int compCount) const;
  };
};
//------------------------------------------------------------------------------


