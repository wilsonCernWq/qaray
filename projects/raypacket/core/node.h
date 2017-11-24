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

#ifndef QARAY_NODE_H
#define QARAY_NODE_H
#pragma once

#include "core/core.h"
#include "core/items.h"
#include "core/transform.h"
#include "core/box.h"
#include "core/hitinfo.h"
#include "math/math.h"

namespace qaray {
//-----------------------------------------------------------------------------
class Node : public ItemBase, public Transformation {
 private:
  Node **child;    // Child nodes
  int numChild;    // The number of child nodes
  Object *obj;    // Object reference
  // (merely points to the object, but does not own the object,
  //  so it doesn't get deleted automatically)
  Material *mtl;  // Material used for shading the object
  Box childBoundBox;  // Bounding box of the child nodes,
  // which does not include the object of this node,
  // but includes the objects of the child nodes
 public:
  Node();
  virtual ~Node();
  // Initialize the node deleting all child nodes
  void Init();
  // Hierarchy management
  int  GetNumChild() const;
  void SetNumChild(int n, int keepOld = false);
  const Node *GetChild(int i) const;
  Node       *GetChild(int i);
  void SetChild(int i, Node *node);
  void AppendChild(Node *node);
  void RemoveChild(int i);
  void DeleteAllChildNodes();
  // Bounding Box
  const Box &ComputeChildBoundBox();
  const Box &GetChildBoundBox() const;
  // Object management
  const Object *GetNodeObj() const;
  Object *GetNodeObj();
  void SetNodeObj(Object *object);
  // Material management
  const Material *GetMaterial() const;
  void SetMaterial(Material *material);
  // Transformations
  Ray ToNodeCoords(const Ray &ray) const;
  DiffRay ToNodeCoords(const DiffRay &ray) const;
  void FromNodeCoords(HitInfo &hInfo) const;
  void FromNodeCoords(DiffHitInfo &hInfo) const;
};
}
#endif //QARAY_NODE_H
