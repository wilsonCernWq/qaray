#include <string>
#include <lodepng.h>
#include "framebuffer.h"

FrameBuffer::FrameBuffer() :
    mask(nullptr),
    img(nullptr),
    zbuffer(nullptr),
    zbufferImg(nullptr),
    sampleCount(nullptr),
    sampleCountImg(nullptr),
    irradComp(nullptr),
    width(0),
    height(0),
    numRenderedPixels(0) {}

FrameBuffer::~FrameBuffer()
{
  if (mask) delete[] mask;
  if (img) delete[] img;
  if (zbuffer) delete[] zbuffer;
  if (zbufferImg) delete[] zbufferImg;
  if (sampleCount) delete[] sampleCount;
  if (sampleCountImg) delete[] sampleCountImg;
  if (irradComp) delete[] irradComp;
}

qaVOID FrameBuffer::Init(qaUINT w, qaUINT h)
{
  width = w;
  height = h;
  if (mask) delete[] mask;
  mask = new qaUCHAR[width * height]();
  if (img) delete[] img;
  img = new Color3c[width * height];
  if (zbuffer) delete[] zbuffer;
  zbuffer = new float[width * height];
  if (zbufferImg) delete[] zbufferImg;
  zbufferImg = nullptr;
  if (sampleCount) delete[] sampleCount;
  sampleCount = new qaUCHAR[width * height];
  if (sampleCountImg) delete[] sampleCountImg;
  sampleCountImg = nullptr;
  if (irradComp) delete[] irradComp;
  irradComp = nullptr;
  ResetNumRenderedPixels();
}

qaVOID FrameBuffer::AllocateIrradianceComputationImage()
{
  if (!irradComp) irradComp = new qaUCHAR[width * height];
  for (qaINT i = 0; i < width * height; i++) irradComp[i] = 0;
}

qaVOID FrameBuffer::ResetNumRenderedPixels()
{
  if (mask) delete[] mask;
  mask = new qaUCHAR[width * height]();
  numRenderedPixels = 0;
}

qaVOID FrameBuffer::ComputeZBufferImage()
{
  qaINT size = width * height;
  if (zbufferImg) delete[] zbufferImg;
  zbufferImg = new qaUCHAR[size];

  float zmin = BIGFLOAT, zmax = 0;
  for (qaINT i = 0; i < size; i++) {
    if (zbuffer[i] == BIGFLOAT) continue;
    if (zmin > zbuffer[i]) zmin = zbuffer[i];
    if (zmax < zbuffer[i]) zmax = zbuffer[i];
  }
  for (qaINT i = 0; i < size; i++) {
    if (zbuffer[i] == BIGFLOAT) zbufferImg[i] = 0;
    else {
      float f = (zmax - zbuffer[i]) / (zmax - zmin);
      qaUCHAR c = qaUCHAR(f * 255);
      if (c < 0) c = 0;
      if (c > 255) c = 255;
      zbufferImg[i] = c;
    }
  }
}

qaINT FrameBuffer::ComputeSampleCountImage()
{
  qaINT size = width * height;
  if (sampleCountImg) delete[] sampleCountImg;
  sampleCountImg = new qaUCHAR[size];
  qaUCHAR smin = 255, smax = 0;
  for (qaINT i = 0; i < size; i++) {
    if (smin > sampleCount[i]) smin = sampleCount[i];
    if (smax < sampleCount[i]) smax = sampleCount[i];
  }
  if (smax == smin) {
    for (qaINT i = 0; i < size; i++) sampleCountImg[i] = 0;
  } else {
    for (qaINT i = 0; i < size; i++) {
      auto c = qaUCHAR((255 * (sampleCount[i] - smin)) / (smax - smin));
      if (c < 0) c = 0;
      if (c > 255) c = 255;
      sampleCountImg[i] = c;
    }
  }
  return smax;
}

qaBOOL FrameBuffer::SavePNG(const qaCHAR *filename,
                            qaUCHAR *data,
                            qaINT compCount) const
{
  LodePNGColorType colortype;
  switch (compCount) {
    case 1:colortype = LCT_GREY;
      break;
    case 3:colortype = LCT_RGB;
      break;
    default:return false;
  }
  qaUINT error = lodepng::encode(filename, data, width, height, colortype, 8);
  return error == 0;
}

qaBOOL FrameBuffer::SaveImage(const qaCHAR *filename) const
{
  return SavePNG(filename, &img[0].r, 3);
}

qaBOOL FrameBuffer::SaveZImage(const qaCHAR *filename) const
{
  return SavePNG(filename, zbufferImg, 1);
}

qaBOOL FrameBuffer::SaveSampleCountImage(const qaCHAR *filename) const
{
  return SavePNG(filename, sampleCountImg, 1);
}

qaBOOL FrameBuffer::SaveIrradianceComputationImage(const qaCHAR *filename) const
{
  return SavePNG(filename, irradComp, 1);
} 
