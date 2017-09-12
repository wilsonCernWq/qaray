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

#include "framebuffer.h"

namespace qw {

  RenderImage::RenderImage() : 
    img(NULL), zbuffer(NULL), zbufferImg(NULL),
    width(0), height(0), numRenderedPixels(0) 
  {}
  void RenderImage::Init(int w, int h)
  {
    width=w;
    height=h;
    if (img) delete [] img;
    img = new Color3c[width*height];
    if (zbuffer) delete [] zbuffer;
    zbuffer = new float[width*height];
    if (zbufferImg) delete [] zbufferImg;
    zbufferImg = NULL;
    ResetNumRenderedPixels();
  }
    
  int RenderImage::GetWidth() const { return width; }
  int RenderImage::GetHeight() const { return height; }
  Color3c* RenderImage::GetPixels() { return img; }
  float*   RenderImage::GetZBuffer() { return zbuffer; }
  uchar*   RenderImage::GetZBufferImage() { return zbufferImg; }
    
  void RenderImage::ResetNumRenderedPixels() { numRenderedPixels=0; }
  int	 RenderImage::GetNumRenderedPixels() const { return numRenderedPixels; }
  void RenderImage::IncrementNumRenderPixel(int n) { numRenderedPixels+=n; }
  bool RenderImage::IsRenderDone() const { return numRenderedPixels >= width*height; }
    
  void RenderImage::ComputeZBufferImage()
  {
    int size = width * height;
    if (zbufferImg) delete [] zbufferImg;
    zbufferImg = new uchar[size];
      
    float zmin=BIGFLOAT, zmax=0;
    for ( int i=0; i<size; i++ ) {
      if ( zbuffer[i] == BIGFLOAT ) continue;
      if ( zmin > zbuffer[i] ) zmin = zbuffer[i];
      if ( zmax < zbuffer[i] ) zmax = zbuffer[i];
    }
    for ( int i=0; i<size; i++ ) {
      if ( zbuffer[i] == BIGFLOAT ) zbufferImg[i] = 0;
      else {
	float f = (zmax-zbuffer[i])/(zmax-zmin);
	int c = int(f * 255);
	if ( c < 0 ) c = 0;
	if ( c > 255 ) c = 255;
	zbufferImg[i] = c;
      }
    }
  }
    
  bool RenderImage::SaveImage (const char *filename) const 
  { 
    return SavePNG(filename,&img[0].r,3); 
  }
  bool RenderImage::SaveZImage(const char *filename) const { 
    return SavePNG(filename,zbufferImg,1); 
  }
   
  bool RenderImage::SavePNG(const char *filename, uchar *data, int compCount) const
  {
    LodePNGColorType colortype;
    switch( compCount ) {
    case 1: colortype = LCT_GREY; break;
    case 3: colortype = LCT_RGB;  break;
    default: return false;
    }
    unsigned int error = lodepng::encode(filename,data,width,height,colortype,8);
    return error == 0;
  }

};
