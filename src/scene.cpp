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

#include "scene.h"
#include <random>
#include <thread>

namespace qaray {

//------------------------------------------------------------------------------
// Trace the ray within this node and all its children
//------------------------------------------------------------------------------
bool Scene::TraceNodeShadow(Node &node, Ray &ray, HitInfo &hInfo)
{
  Ray nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()
        ->IntersectRay(nodeRay, hInfo, HIT_FRONT_AND_BACK)) { return true; }
  }
  for (int c = 0; c < node.GetNumChild(); ++c) {
    if (TraceNodeShadow(*(node.GetChild(c)), nodeRay, hInfo)) { return true; }
  }
  return false;
}

//------------------------------------------------------------------------------
// Trace the ray within this node and all its children
//------------------------------------------------------------------------------
bool Scene::TraceNodeNormal(Node &node, DiffRay &ray,
                            DiffHitInfo &hInfo /* it stores results */)
{
  bool hasHit = false;
  // We first check if this ray will intersect with the object held by
  // this node
  DiffRay nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()->IntersectRay(nodeRay.c, hInfo.c,
                                        HIT_FRONT_AND_BACK,
                                        &nodeRay, &hInfo)) {
      hInfo.c.node = &node;
      hasHit = true;
    }
  }
  // Now we still need to check all the children contained by this node
  for (int c = 0; c < node.GetNumChild(); ++c) {
    // Reaches here means this node has at lease one child
    if (TraceNodeNormal(*(node.GetChild(c)), nodeRay, hInfo)) {
      hasHit = true;
    }
  }
  if (hasHit) { node.FromNodeCoords(hInfo); }
  return hasHit;
}
///--------------------------------------------------------------------------//
Scene scene;
///--------------------------------------------------------------------------//
RenderImage renderImage;
///--------------------------------------------------------------------------//
ThreadSampler *rng = new ThreadSampler(Sampler_Marsaglia());
///--------------------------------------------------------------------------//
}
//------------------------------------------------------------------------------

SuperSamplerHalton::SuperSamplerHalton(const Color3f th,
                                       const int sppMin,
                                       const int sppMax)
    : th(th), sppMin(sppMin), sppMax(sppMax) {}

const Color3f &SuperSamplerHalton::GetColor() const { return color; }

int SuperSamplerHalton::GetSampleID() const { return s; }

bool SuperSamplerHalton::Loop() const
{
  return (s < sppMin ||
      (s >= sppMin && s < sppMax &&
          (color_std.r > th.r || color_std.g > th.g || color_std.b > th.b)));
}

Point3 SuperSamplerHalton::NewPixelSample()
{
  return Point3(Halton(s, 11), Halton(s, 13), 0.f);
}

Point3 SuperSamplerHalton::NewDofSample(const float R)
{
  float r1, r2;
  rng->local().Get2f(r1, r2);
  const float r = R * SQRT(r1);
  const float t = r2 * 2.f * PI;
  return Point3(r * COS(t), r * SIN(t), 0.f);
}

void SuperSamplerHalton::Accumulate(const Color3f &localColor)
{
  const Color3f dc = (localColor - color) / static_cast<float>(s + 1);
  color += dc;
  color_std += s > 0 ?
               dc * dc * static_cast<float>(s + 1)
                   - color_std / static_cast<float>(s) :
               Color3f(0.0f);
}

void SuperSamplerHalton::Increment() { ++s; }

//------------------------------------------------------------------------------
