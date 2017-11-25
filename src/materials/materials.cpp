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

int Material::maxBounce = 1;

//------------------------------------------------------------------------------

Color3f Attenuation(const Color3f &absorption, const float l)
{
  const float R = exp(-absorption.r * l);
  const float G = exp(-absorption.g * l);
  const float B = exp(-absorption.b * l);
  return Color3f(R, G, B); // attenuation
}

//------------------------------------------------------------------------------

