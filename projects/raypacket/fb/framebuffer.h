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

//------------------------------------------------------------------------------

class RenderImage
{
private:
  Color24 *img;
  float   *zbuffer;
  uchar   *zbufferImg;
  int		width, height;
  std::atomic<int> numRenderedPixels;
public:
RenderImage() : img(NULL), zbuffer(NULL), zbufferImg(NULL), width(0), height(0), numRenderedPixels(0) {}
  void Init(int w, int h)
  {
    width=w;
    height=h;
    if (img) delete [] img;
    img = new Color24[width*height];
    if (zbuffer) delete [] zbuffer;
    zbuffer = new float[width*height];
    if (zbufferImg) delete [] zbufferImg;
    zbufferImg = NULL;
    ResetNumRenderedPixels();
  }

  int			GetWidth() const	{ return width; }
  int			GetHeight() const	{ return height; }
  Color24*	GetPixels()			{ return img; }
  float*		GetZBuffer()		{ return zbuffer; }
  uchar*		GetZBufferImage()	{ return zbufferImg; }

  void	ResetNumRenderedPixels()		{ numRenderedPixels=0; }
  int		GetNumRenderedPixels() const	{ return numRenderedPixels; }
  void	IncrementNumRenderPixel(int n)	{ numRenderedPixels+=n; }
  bool	IsRenderDone() const			{ return numRenderedPixels >= width*height; }

  void	ComputeZBufferImage()
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

  bool SaveImage (const char *filename) const { return SavePNG(filename,&img[0].r,3); }
  bool SaveZImage(const char *filename) const { return SavePNG(filename,zbufferImg,1); }

private:
  bool SavePNG(const char *filename, uchar *data, int compCount) const
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

//------------------------------------------------------------------------------


