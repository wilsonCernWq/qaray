//------------------------------------------------------------------------------
///
/// \file       globalvar.cpp
/// \author     Qi WU
/// \version    2.0
/// \date       August 28, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "globalvar.h"
#include <chrono>      /* C11 */
#include <iostream>

//------------------------------------------------------------------------------
void TimeFrame(TimeState state)
{
  static float avgRenderTime = 0.0f;
  static std::chrono::time_point<std::chrono::system_clock> startTime, endTime;
  static int numFrames = -1; // don't count the first frame
  if (state == START_FRAME) {
    startTime = std::chrono::system_clock::now();
  } else if (state == STOP_FRAME) {
    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    printf("\nRender time is %f s\n", elapsed_seconds.count());
    if (++numFrames > 0) { // Update moving average
      avgRenderTime += (elapsed_seconds.count() - avgRenderTime) / numFrames;
    }
  } else {
    printf("\nEnding the program, average frame time is %f s\n\n",
           avgRenderTime);
  }
}

//------------------------------------------------------------------------------
// unique
//Node rootNode;
//Camera camera;
//RenderImage renderImage;
//MaterialList materials;
//LightList lights;
//ObjFileList objList;
//TexturedColor background;
//TexturedColor environment;
//TextureList textureList;

//------------------------------------------------------------------------------
