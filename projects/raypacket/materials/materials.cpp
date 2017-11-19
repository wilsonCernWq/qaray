//------------------------------------------------------------------------------
///
/// \file       materials.cpp
/// \author     Qi WU
/// \version    1.0
/// \date       September, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "materials.h"
#include "globalvar.h"

int Material::maxBounce   = 1;
int Material::maxBounceMC = 1;
int Material::maxMCSample = 8;
float Material::gamma = 1.0;
bool  Material::sRGB = false;

//------------------------------------------------------------------------------

float LinearToSRGB(const float c) {
  if (!Material::sRGB) { return c; }
  const float a = 0.055f;
  if (c < 0.0031308f) { return 12.92f * c; }
  else { return (1.f + a) * POW(c, 1.f/2.4f) - a; }
}

Color Attenuation(const Color& absorption, const float l) {
  const float R = exp(-absorption.r * l);
  const float G = exp(-absorption.g * l);
  const float B = exp(-absorption.b * l);
  return Color(R, G, B); // attenuation
}

//------------------------------------------------------------------------------

