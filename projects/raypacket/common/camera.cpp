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

#include "camera.h"

namespace qw {
  Camera::Camera(): 
    pos(0,0, 0), 
    dir(0,0,-1), 
    up (0,1, 0),
    fov(40.f), 
    imgWidth(200), 
    imgHeight(150)
  {}
};
