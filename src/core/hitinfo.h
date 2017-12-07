///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/24/17.                                             //
/// Copyright (c) 2017 University of Utah. All rights reserved.             //
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

#ifndef QARAY_HITINFO_H
#define QARAY_HITINFO_H
#pragma once

#include "core/setup.h"
#include "math/math.h"

namespace qaray {
class HitInfo {
 public:
  float z;            // the distance from the ray center to the hit point
  Point3 p;           // position of the hit point
  Point3 N;           // surface normal at the hit point
  Point3 uvw;         // texture coordinate at the hit point
  Point3 duvw[2];     // derivatives of the texture coordinate
  int mtlID;          // sub-material index
  const Node *node;   // the object node that was hit, false if the ray hits the back side
  bool hasFrontHit;   // true if the ray hits the front side,
  bool hasTexture;
  bool hasDiffuseHit;
 public:
  HitInfo() { Init(); }
  void Init();
};
class HitInfoCore {
 public:
  float z;  // the distance from the ray center to the hit point
  Point3 p; // position of the hit point
  Point3 N; // surface normal at the hit point
 public:
  HitInfoCore() { Init(); }
  void Init() { z = BIGFLOAT; }
};
class DiffHitInfo {
 public:
  HitInfo c;
  HitInfoCore x, y;
 public:
  DiffHitInfo() { Init(); }
  void Init()
  {
    c.Init();
    x.Init();
    y.Init();
  }
};
}

#endif //QARAY_HITINFO_H
