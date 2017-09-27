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
(Node& node, Ray& ray, HitInfo& hInfo /* it stores results */) 
{
  bool hasHit = false;
  // We first check if this ray will intersect with the object held by
  // this node
  Ray nodeRay = node.ToNodeCoords(ray);
  if (node.GetNodeObj() != NULL) {
    if (node.GetNodeObj()->IntersectRay(nodeRay, hInfo, HIT_FRONT_AND_BACK)) {
      hInfo.node = &node; hasHit = true;
    }
  }  
  // Now we still need to check all the childs contained by this node
  for (size_t c = 0; c < node.GetNumChild(); ++c) {
    // Reaches here means this node has at lease one child
    if (TraceNodeNormal(*(node.GetChild(c)), nodeRay, hInfo)) { hasHit = true; }   
  }
  if (hasHit) { node.FromNodeCoords(hInfo); }
  return hasHit;
}

//------------------------------------------------------------------------------
