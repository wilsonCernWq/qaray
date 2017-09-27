//------------------------------------------------------------------------------
///
/// \file       globalvar.h
/// \author     Qi WU
/// \version    2.0
/// \date       August 28, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#pragma once
#ifndef _GLOBALVAR_H_
#define _GLOBALVAR_H_

#include "scene.h"
#include "objects.h"
#include "materials.h"
#include "lights.h"

enum TimeState {START_FRAME, STOP_FRAME, KILL_FRAME};

//------------------------------------------------------------------------------
void TimeFrame(TimeState state);
//------------------------------------------------------------------------------
// unique
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;
extern MaterialList materials;
extern LightList lights;
extern ObjFileList objList;
// objects
extern Sphere theSphere;
extern Plane thePlane;
//------------------------------------------------------------------------------

#endif//_GLOBALVAR_H_
