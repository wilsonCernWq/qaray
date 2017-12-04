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

#ifndef QARAY_VIEWPORT_H
#define QARAY_VIEWPORT_H
#pragma once

#include "renderers/Renderer_GUI.h"

//! Register Renderer
void RegisterRenderer(Renderer_GUI& r);

// Show OpenGL PreView Window
void ShowViewport();

#endif//QARAY_VIEWPORT_H
