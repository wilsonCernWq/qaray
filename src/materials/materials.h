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
//-----------------------------------------------------------------------------
///
/// \file       materials.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 11, 2015
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-----------------------------------------------------------------------------
#ifndef QARAY_MATERIALS_H
#define QARAY_MATERIALS_H
#pragma once

#include "scene/scene.h"

//-----------------------------------------------------------------------------

Color3f Attenuation(const Color3f &absorption, float l);

//-----------------------------------------------------------------------------

#include "materials/MtlBlinn_PhotonMap.h"
#include "materials/MtlBlinn_PathTracing.h"
#include "materials/MtlBlinn_MonteCarloGI.h"
#include "materials/MtlBlinn_Basic.h"
#include "materials/MtlPhong_Basic.h"

//-----------------------------------------------------------------------------

//using MtlBlinn = MtlBlinn_PhotonMap;
using MtlBlinn = MtlBlinn_PathTracing;
//using MtlBlinn = MtlBlinn_MonteCarloGI;
//using MtlBlinn = MtlBlinn_Basic;
//using MtlBlinn = MtlPhong_Basic;

//-----------------------------------------------------------------------------

class MultiMtl : public Material {
 public:

  ~MultiMtl() override { for (auto &m : mtls) delete m; }

  Color3f Shade(const DiffRay &ray, const DiffHitInfo &hInfo,
                const LightList &lights, int bounceCount) const override
  {
    return hInfo.c.mtlID < (int) mtls.size() ?
           mtls[hInfo.c.mtlID]->Shade(ray, hInfo, lights, bounceCount) :
           Color3f(1, 1, 1);
  }

  void SetViewportMaterial(int subMtlID) const override
  {
    if (subMtlID < (int) mtls.size()) mtls[subMtlID]->SetViewportMaterial(0);
  }

  void AppendMaterial(Material *m) { mtls.push_back(m); }

 private:
  std::vector<Material *> mtls;
};

//-----------------------------------------------------------------------------

#endif//QARAY_MATERIALS_H
