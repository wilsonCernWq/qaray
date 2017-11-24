#include <string>
#include <lodepng.h>
#include "framebuffer.h"

RenderImage::RenderImage() :
    mask(NULL),
    img(NULL),
    zbuffer(NULL),
    zbufferImg(NULL),
    sampleCount(NULL),
    sampleCountImg(NULL),
    irradComp(NULL),
    width(0),
    height(0),
    numRenderedPixels(0) {}

RenderImage::~RenderImage()
{
  if (mask) delete[] mask;
  if (img) delete[] img;
  if (zbuffer) delete[] zbuffer;
  if (zbufferImg) delete[] zbufferImg;
  if (irradComp) delete[] irradComp;
}

void RenderImage::Init(int w, int h)
{
  width = w;
  height = h;
  if (mask) delete[] mask;
  mask = new uchar[width * height]();
  if (img) delete[] img;
  img = new Color24[width * height];
  if (zbuffer) delete[] zbuffer;
  zbuffer = new float[width * height];
  if (zbufferImg) delete[] zbufferImg;
  zbufferImg = NULL;
  if (sampleCount) delete[] sampleCount;
  sampleCount = new uchar[width * height];
  if (sampleCountImg) delete[] sampleCountImg;
  sampleCountImg = NULL;
  if (irradComp) delete[] irradComp;
  irradComp = NULL;
  ResetNumRenderedPixels();
}

void RenderImage::AllocateIrradianceComputationImage()
{
  if (!irradComp) irradComp = new uchar[width * height];
  for (int i = 0; i < width * height; i++) irradComp[i] = 0;
}

void RenderImage::ResetNumRenderedPixels()
{
  if (mask) delete[] mask;
  mask = new uchar[width * height]();
  numRenderedPixels = 0;
}

void RenderImage::ComputeZBufferImage()
{
  int size = width * height;
  if (zbufferImg) delete[] zbufferImg;
  zbufferImg = new uchar[size];

  float zmin = BIGFLOAT, zmax = 0;
  for (int i = 0; i < size; i++) {
    if (zbuffer[i] == BIGFLOAT) continue;
    if (zmin > zbuffer[i]) zmin = zbuffer[i];
    if (zmax < zbuffer[i]) zmax = zbuffer[i];
  }
  for (int i = 0; i < size; i++) {
    if (zbuffer[i] == BIGFLOAT) zbufferImg[i] = 0;
    else {
      float f = (zmax - zbuffer[i]) / (zmax - zmin);
      int c = int(f * 255);
      if (c < 0) c = 0;
      if (c > 255) c = 255;
      zbufferImg[i] = c;
    }
  }
}

int RenderImage::ComputeSampleCountImage()
{
  int size = width * height;
  if (sampleCountImg) delete[] sampleCountImg;
  sampleCountImg = new uchar[size];
  uchar smin = 255, smax = 0;
  for (int i = 0; i < size; i++) {
    if (smin > sampleCount[i]) smin = sampleCount[i];
    if (smax < sampleCount[i]) smax = sampleCount[i];
  }
  if (smax == smin) {
    for (int i = 0; i < size; i++) sampleCountImg[i] = 0;
  } else {
    for (int i = 0; i < size; i++) {
      int c = (255 * (sampleCount[i] - smin)) / (smax - smin);
      if (c < 0) c = 0;
      if (c > 255) c = 255;
      sampleCountImg[i] = c;
    }
  }
  return smax;
}

bool RenderImage::SavePNG(const char *filename,
                          uchar *data,
                          int compCount) const
{
  LodePNGColorType colortype;
  switch (compCount) {
    case 1:colortype = LCT_GREY;
      break;
    case 3:colortype = LCT_RGB;
      break;
    default:return false;
  }
  unsigned int
      error = lodepng::encode(filename, data, width, height, colortype, 8);
  return error == 0;
}

bool RenderImage::SaveImage(const char *filename) const
{
  return SavePNG(filename,
                 &img[0].r,
                 3);
}

bool RenderImage::SaveZImage(const char *filename) const
{
  return SavePNG(filename,
                 zbufferImg,
                 1);
}

bool RenderImage::SaveSampleCountImage(const char *filename) const
{
  return SavePNG(filename, sampleCountImg, 1);
}

bool RenderImage::SaveIrradianceComputationImage(const char *filename) const
{
  return SavePNG(filename, irradComp, 1);
} 
