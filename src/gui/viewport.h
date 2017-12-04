//------------------------------------------------------------------------------
///
/// \file       viewport.h
/// \author     Qi WU
/// \version    2.0
/// \date       August 28, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#pragma once
#ifndef _VIEWPORT_H_
#define _VIEWPORT_H_

// Called to start rendering (renderer must run in a separate thread)
void BeginRender();

// Called to end rendering (if it is not already finished)
void StopRender();

// Called when the rendering is end successfully
void CleanRender();

// Called when the program is stopped
void KillRender();

// Show OpenGL PreView Window
void ShowViewport();

#endif//_VIEWPORT_H_
