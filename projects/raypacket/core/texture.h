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

#ifndef QARAY_TEXTURE_H
#define QARAY_TEXTURE_H
#pragma once

#include "core/setup.h"
#include "core/items.h"
#include "core/transform.h"
#include "math/math.h"

namespace qaray {
class Texture : public ItemBase {
 public:
  // Evaluates the color at the given uvw location.
  virtual Color3f Sample(const Point3 &uvw) const = 0;
  // Evaluates the color around the given uvw location using the derivatives
  // duvw by calling the Sample function multiple times.
  virtual Color3f Sample(const Point3 &uvw,
                         const Point3 duvw[2],
                         bool elliptic) const;
  // Used for OpenGL display
  virtual bool SetViewportTexture() const { return false; }
 protected:
  // Clamps the uvw values for tiling textures, such that all values fall
  // between 0 and 1.
  static Point3 TileClamp(const Point3 &uvw);
};
typedef ItemFileList<Texture> TextureList;
//-----------------------------------------------------------------------------
// This class handles textures with texture transformations.
// The uvw values passed to the Sample methods are transformed
// using the texture transformation.
class TextureMap : public Transformation {
 public:
  TextureMap() : texture(nullptr) {}
  explicit TextureMap(Texture *tex) : texture(tex) {}
  void SetTexture(Texture *tex) { texture = tex; }
  virtual Color3f Sample(const Point3 &uvw) const;
  virtual Color3f Sample(const Point3 &uvw,
                         const Point3 duvw[2],
                         bool elliptic) const;
  // Used for OpenGL display
  bool SetViewportTexture() const;
 private:
  Texture *texture;
};
//-----------------------------------------------------------------------------
// This class keeps a TextureMap and a color. This is useful for keeping
// material color parameters that can also be textures. If no texture is
// specified, it automatically uses the color value. Otherwise, the texture
// value is multiplied by the color value.
class TexturedColor {
 private:
  Color3f color;
  TextureMap *map;
 public:
  TexturedColor() : color(0, 0, 0), map(nullptr) {}
  TexturedColor(float r, float g, float b) : color(r, g, b), map(nullptr) {}
  virtual ~TexturedColor() { if (map) { delete map; }}
  void SetColor(const Color3f &c) { color = c; }
  void SetTexture(TextureMap *m);
  Color3f GetColor() const { return color; }
  const TextureMap *GetTexture() const { return map; }
  Color3f Sample(const Point3 &uvw) const;
  Color3f Sample(const Point3 &uvw,
                 const Point3 duvw[2],
                 bool elliptic = true) const;
  // Returns the color value at the given direction for environment mapping.
  Color3f SampleEnvironment(const Point3 &dir) const;
};
}

#endif //QARAY_TEXTURE_H
