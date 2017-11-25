///--------------------------------------------------------------------------//
///                                                                          //
/// Copyright(c) 2017-2018, Qi WU (University of Utah)                       //
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

#ifndef QARAY_SAMPLER_MARSAGLIA_H
#define QARAY_SAMPLER_MARSAGLIA_H
#pragma once

#include "core/sampler.h"
#include "math/math.h"

namespace qaray {
class Sampler_Marsaglia : public Sampler {
 private:
  bool initialized = false;
  qaUINT seed[4] = {1};
  qaFLOAT xorshift32 (qaUINT state[4]);
  qaFLOAT xorshift128(qaUINT state[4]);
  void Init();
 public:
  void Get1f(qaFLOAT &r1) override;
  void Get2f(qaFLOAT &r1, qaFLOAT &r2) override;
  void Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3) override;
};
}

#endif //QARAY_SAMPLER_MARSAGLIA_H

