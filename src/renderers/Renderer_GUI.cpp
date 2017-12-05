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

#include "Renderer_GUI.h"
#include "gui/viewport.h"

namespace qaray {
Renderer_GUI::Renderer_GUI(RendererParam& param): Renderer(param)
{ RegisterRenderer(*this); }
//---------------------------------------------------------------------------//
// Called to start rendering (renderer must run in a separate thread)
//---------------------------------------------------------------------------//
void Renderer_GUI::BeginRender()
{
  // Reset
  StopRender();
  renderImage->ResetNumRenderedPixels();
  // Start threads
  threadStop = false;
  threadMain = new std::thread([&]{
    this->ThreadRender();
  });
}
//---------------------------------------------------------------------------//
// Called to end rendering (if it is not already finished)
//---------------------------------------------------------------------------//
void Renderer_GUI::StopRender()
{
  // Send stop signal
  threadStop = true;
  // Wait for threads to finish
  if (threadMain != nullptr) {
    threadMain->join();
    delete threadMain;
    threadMain = nullptr;
  }
}
//---------------------------------------------------------------------------//
// Called when the rendering is end successfully
//---------------------------------------------------------------------------//
void Renderer_GUI::CleanRender()
{
  // Save image
  renderImage->ComputeZBufferImage();
  renderImage->ComputeSampleCountImage();
  renderImage->SaveImage("colorBuffer.png");
  renderImage->SaveZImage("depthBuffer.png");
  renderImage->SaveSampleCountImage("sampleBuffer.png");
}
//---------------------------------------------------------------------------//
// Called when the program is stopped
//---------------------------------------------------------------------------//
void Renderer_GUI::KillRender()
{
  StopRender();
  TimeFrame(KILL_FRAME);
}
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
void Renderer_GUI::Render()
{
  //-------------------------------------------------------------------------//
  //
  //-------------------------------------------------------------------------//
#ifdef USE_GUI
  ShowViewport();
#else
  std::cerr << "Warning: GUI Mode is not enabled. "
               "Add '-DENABLE_GUI' to the cmake and recompile the program"
            << std::endl;
#endif
}
}
