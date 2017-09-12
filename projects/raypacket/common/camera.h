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
/// \file       camera.h 
/// \author     Qi WU
///
/// \brief Modified for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "math/math.h"

//------------------------------------------------------------------------------

namespace qw {
  class Camera {
  public:
    vec3f pos, dir, up;
    float  fov;
    size_t imgWidth, imgHeight;
    Camera();
  };
}

//------------------------------------------------------------------------------
