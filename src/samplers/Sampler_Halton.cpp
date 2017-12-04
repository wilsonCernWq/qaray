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

#include <ctime>
#include <cstdlib>
#include "Sampler_Halton.h"

namespace qaray {
void Sampler_Halton::Init()
{
  if (!initialized) {
    printf("Init\n");
    /* The seed word must be initialized to non-zero */
    srand(static_cast<unsigned int>(time(nullptr)));
    seed = static_cast<qaUINT>(rand()) % 256 + 1;
    initialized = true;
  }
}
qaFLOAT Sampler_Halton::sample(qaINT index, qaINT base)
{
  qaFLOAT r = 0;
  qaFLOAT f = 1.0f / (qaFLOAT) base;
  for (qaINT i = index; i > 0; i /= base) {
    r += f * (i % base);
    f /= (qaFLOAT) base;
  }
  return r;
}
void Sampler_Halton::Get1f(qaFLOAT &r1)
{
  Init();
  r1 = sample(seed++, 2);
}
void Sampler_Halton::Get2f(qaFLOAT &r1, qaFLOAT &r2)
{
  Init();
  r1 = sample(seed, 2);
  r2 = sample(seed, 3);
  ++seed;
}
void Sampler_Halton::Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3)
{
  Init();
  r1 = sample(seed, 2);
  r2 = sample(seed, 3);
  r3 = sample(seed, 5);
  ++seed;
}
}