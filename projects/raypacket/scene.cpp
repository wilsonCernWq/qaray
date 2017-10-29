//------------------------------------------------------------------------------
///
/// \file       objects.cpp 
/// \author     Qi WU
/// \version    1.0
/// \date       August, 2017
///
/// \brief Source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#include "scene.h"

const float DiffRay::dx = 0.01f;
const float DiffRay::dy = 0.01f;
const float DiffRay::rdx = 1.f/DiffRay::dx;
const float DiffRay::rdy = 1.f/DiffRay::dy;

//------------------------------------------------------------------------------
// Trace the ray within this node and all its children
//------------------------------------------------------------------------------

bool TraceNodeShadow
    (Node& node, Ray& ray, HitInfo& hInfo)
{
  Ray nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()->IntersectRay(nodeRay, hInfo, HIT_FRONT_AND_BACK)) { return true; }
  }
  for (size_t c = 0; c < node.GetNumChild(); ++c) {
    if (TraceNodeShadow(*(node.GetChild(c)), nodeRay, hInfo)) { return true; }
  }
  return false;
}

//------------------------------------------------------------------------------

bool TraceNodeNormal
    (Node& node, DiffRay& ray, DiffHitInfo& hInfo /* it stores results */)
{
  bool hasHit = false;
  // We first check if this ray will intersect with the object held by
  // this node
  DiffRay nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()->IntersectRay(nodeRay.c, hInfo.c, HIT_FRONT_AND_BACK,
                                        &nodeRay, &hInfo))
    {
      hInfo.c.node = &node; hasHit = true;
    }
  }
  // Now we still need to check all the childs contained by this node
  for (int c = 0; c < node.GetNumChild(); ++c) {
    // Reaches here means this node has at lease one child
    if (TraceNodeNormal(*(node.GetChild(c)), nodeRay, hInfo)) { hasHit = true; }
  }
  if (hasHit) { node.FromNodeCoords(hInfo); }
  return hasHit;
}

//------------------------------------------------------------------------------

SuperSamplerHalton::SuperSamplerHalton(const Color th, const int sppMin, const int sppMax) 
  : th(th), sppMin(sppMin), sppMax(sppMax) {}
const Color& SuperSamplerHalton::GetColor() const { return color; }
const int SuperSamplerHalton::GetSampleID() const { return s; }
bool SuperSamplerHalton::Loop() const {
  return (s < sppMin ||
	  (s >= sppMin && s <  sppMax &&
	   (color_std.r > th.r || color_std.g > th.g || color_std.b > th.b)));
}
Point3 SuperSamplerHalton::NewPixelSample() {
  return Point3(Halton(s, 2), Halton(s, 3), 0.f);
}
Point3 SuperSamplerHalton::NewDofSample(const float R) {
  const float r = R * SQRT(Halton(s, 5));
  const float t = Halton(s, 7) * 2.f * M_PI;
  return Point3(r * cos(t), r * sin(t), 0.f);
}
void SuperSamplerHalton::Accumulate(const Color& localColor) {
  const Color dc  = (localColor - color) / static_cast<float>(s + 1);
  color += dc;  
  color_std += s > 0 ?
    dc * dc * static_cast<float>(s+1) - color_std / static_cast<float>(s) :
    Color(0.0f);
}
void SuperSamplerHalton::Increment() { ++s; }
