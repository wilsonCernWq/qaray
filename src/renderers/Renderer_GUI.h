///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/3/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.              //
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

#ifndef QARAY_RENDER_GUI_H
#define QARAY_RENDER_GUI_H
#pragma once

#include <thread>
#include "renderers/renderer.h"

namespace qaray {
class Renderer_GUI : public Renderer {
 private:
  std::thread *threadMain = nullptr;
 public:
  Renderer_GUI(RendererParam& param);
  //-------------------------------------------------------------------------//
  // Called to start rendering (renderer must run in a separate thread)
  //-------------------------------------------------------------------------//
  void BeginRender();
  //-------------------------------------------------------------------------//
  // Called to end rendering (if it is not already finished)
  //-------------------------------------------------------------------------//
  void StopRender();
  //-------------------------------------------------------------------------//
  // Called when the rendering is end successfully
  //-------------------------------------------------------------------------//
  void CleanRender();
  //-------------------------------------------------------------------------//
  // Called when the program is stopped
  //-------------------------------------------------------------------------//
  void KillRender();
  //-------------------------------------------------------------------------//
  void Render() override;
};
}

#endif //QARAY_RENDER_GUI_H
