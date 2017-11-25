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

#include <ctime>
#include <cstdlib>
#include "Sampler_Marsaglia.h"

namespace qaray {
void Sampler_Marsaglia::Init() {
  if (!initialized) {
    /* The seed word must be initialized to non-zero */
    srand (static_cast<unsigned int>(time(nullptr)));
    seed[0] = static_cast<qaUINT>(rand()) % 999999999 + 1;
    seed[1] = static_cast<qaUINT>(rand()) % 999999999 + 1;
    seed[2] = static_cast<qaUINT>(rand()) % 999999999 + 1;
    seed[3] = static_cast<qaUINT>(rand()) % 999999999 + 1;
    initialized = true;
  }
}
qaFLOAT Sampler_Marsaglia::xorshift32(qaUINT state[4])
{
  Init();
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  auto x = state[0];
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  state[0] = x;
  return x / (qaFLOAT) (pow(2, 32) - 1);
}
qaFLOAT Sampler_Marsaglia::xorshift128(qaUINT state[4])
{
  Init();
  /* Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs" */
  uint32_t s, t = state[3];
  t ^= t << 11;
  t ^= t >> 8;
  state[3] = state[2]; state[2] = state[1]; state[1] = s = state[0];
  t ^= s;
  t ^= s >> 19;
  state[0] = t;
  return t / (qaFLOAT) (pow(2, 32) - 1);
}
void Sampler_Marsaglia::Get1f(qaFLOAT &r1)
{
  r1 = xorshift32(seed);
}
void Sampler_Marsaglia::Get2f(qaFLOAT &r1, qaFLOAT &r2)
{
  r1 = xorshift32(seed);
  r2 = xorshift32(seed);
}
void Sampler_Marsaglia::Get3f(qaFLOAT &r1, qaFLOAT &r2, qaFLOAT &r3)
{
  r1 = xorshift32(seed);
  r2 = xorshift32(seed);
  r3 = xorshift32(seed);
}
}