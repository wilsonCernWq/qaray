//-------------------------------------------------------------------------------
///
/// \file       texture.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       October 7, 2013
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------

#ifndef _TEXTURE_H_INCLUDED_
#define _TEXTURE_H_INCLUDED_

#include "scene.h"

//-------------------------------------------------------------------------------

class TextureFile : public Texture {
 public:
  TextureFile() : width(0), height(0), viewportTextureID(0) {}

  bool Load();

  virtual Color3f Sample(const Point3 &uvw) const;

  virtual bool SetViewportTexture() const;

 private:
  std::vector<Color3c> data;
  int width, height;
  mutable unsigned int viewportTextureID;
};

//-------------------------------------------------------------------------------

class TextureChecker : public Texture {
 public:
  TextureChecker() : color1(0, 0, 0), color2(1, 1, 1), viewportTextureID(0) {}

  void SetColor1(const Color3f &c) { color1 = c; }

  void SetColor2(const Color3f &c) { color2 = c; }

  virtual Color3f Sample(const Point3 &uvw) const;

  virtual bool SetViewportTexture() const;

 private:
  Color3f color1, color2;
  mutable unsigned int viewportTextureID;
};

//-------------------------------------------------------------------------------

#endif
