///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/24/17.                                             //
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

#include "texture.h"
#include "core/sampler.h"

namespace qaray {
Color3f Texture::Sample(const Point3 &uvw,
                        const Point3 duvw[2],
                        bool elliptic) const
{
  Color3f c = Sample(uvw);
  if (length2(duvw[0]) + length2(duvw[1]) == 0) return c;
  for (int i = 1; i < TEXTURE_SAMPLE_COUNT; i++) {
    float x = Halton(i, 2);
    float y = Halton(i, 3);
    if (elliptic) {
      float r = sqrtf(x) * 0.5f;
      x = r * sinf(y * (float) M_PI * 2);
      y = r * cosf(y * (float) M_PI * 2);
    } else {
      if (x > 0.5f) x -= 1;
      if (y > 0.5f) y -= 1;
    }
    c += Sample(uvw + x * duvw[0] + y * duvw[1]);
  }
  return c / float(TEXTURE_SAMPLE_COUNT);
}
Point3 Texture::TileClamp(const Point3 &uvw)
{
  Point3 u;
  u.x = uvw.x - (int) uvw.x;
  u.y = uvw.y - (int) uvw.y;
  u.z = uvw.z - (int) uvw.z;
  if (u.x < 0) u.x += 1;
  if (u.y < 0) u.y += 1;
  if (u.z < 0) u.z += 1;
  return u;
}
//-----------------------------------------------------------------------------
Color3f TextureMap::Sample(const Point3 &uvw) const
{
  return texture ? texture->Sample(TransformTo(uvw)) : Color3f(0, 0, 0);
}
Color3f TextureMap::Sample(const Point3 &uvw,
                           const Point3 duvw[2],
                           bool elliptic)
const
{
  if (texture == nullptr) return Color3f(0, 0, 0);
  Point3 u = TransformTo(uvw);
  Point3 d[2];
  d[0] = TransformTo(duvw[0] + uvw) - u;
  d[1] = TransformTo(duvw[1] + uvw) - u;
  return texture->Sample(u, d, elliptic);
}
// Used for OpenGL display
bool TextureMap::SetViewportTexture() const
{
  if (texture) return texture->SetViewportTexture();
  return false;
}
//-----------------------------------------------------------------------------
void TexturedColor::SetTexture(TextureMap *m)
{
  if (map) { delete map; }
  map = m;
}
Color3f TexturedColor::Sample(const Point3 &uvw) const
{
  return (map) ? color * map->Sample(uvw) : color;
}
Color3f TexturedColor::Sample(const Point3 &uvw,
                              const Point3 duvw[2],
                              bool elliptic)
const
{
  return (map) ? color * map->Sample(uvw, duvw, elliptic) : color;
}
// Returns the color value at the given direction for environment mapping.
Color3f TexturedColor::SampleEnvironment(const Point3 &dir) const
{
  float z = asinf(-dir.z) / float(M_PI) + 0.5f;
  float x = dir.x / (ABS(dir.x) + ABS(dir.y));
  float y = dir.y / (ABS(dir.x) + ABS(dir.y));
  return Sample(Point3(0.5f, 0.5f, 0.0f) + z
      * (x * Point3(0.5f, 0.5f, 0) + y * Point3(-0.5f, 0.5f, 0)));
}
}