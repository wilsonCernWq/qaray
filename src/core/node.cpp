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

#include "node.h"
#include "core/object.h"

namespace qaray {
Node::Node() :
    child(nullptr),
    numChild(0),
    obj(nullptr),
    mtl(nullptr) {}
Node::~Node() { DeleteAllChildNodes(); }
// Initialize the node deleting all child nodes
void Node::Init()
{
  DeleteAllChildNodes();
  obj = NULL;
  mtl = NULL;
  SetName(NULL);
  InitTransform();
}
// Hierarchy management
int Node::GetNumChild() const { return numChild; }
void Node::SetNumChild(int n, int keepOld)
{
  if (n < 0) n = 0;  // just to be sure
  Node **nc = nullptr;  // new child pointer
  if (n > 0) {
    nc = new Node *[n];
    for (int i = 0; i < n; i++) nc[i] = nullptr;
  }
  if (keepOld) {
    int sn = MIN(n, numChild);
    for (int i = 0; i < sn; i++) nc[i] = child[i];
  }
  if (child) delete[] child;
  child = nc;
  numChild = n;
}
const Node* Node::GetChild(int i) const { return child[i]; }
Node* Node::GetChild(int i) { return child[i]; }
void Node::SetChild(int i, Node *node) { child[i] = node; }
void Node::AppendChild(Node *node)
{
  SetNumChild(numChild + 1, true);
  SetChild(numChild - 1, node);
}
void Node::RemoveChild(int i)
{
  for (int j = i; j < numChild - 1; j++) child[j] = child[j + 1];
  SetNumChild(numChild - 1);
}
void Node::DeleteAllChildNodes()
{
  for (int i = 0; i < numChild; i++) {
    child[i]->DeleteAllChildNodes();
    delete child[i];
  }
  SetNumChild(0);
}
// Bounding Box
const Box& Node::ComputeChildBoundBox()
{
  childBoundBox.Init();
  for (int i = 0; i < numChild; i++) {
    Box childBox = child[i]->ComputeChildBoundBox();
    Object *cobj = child[i]->GetNodeObj();
    if (cobj) childBox += cobj->GetBoundBox();
    if (!childBox.IsEmpty()) {
      // transform the box from child coordinates
      for (int j = 0; j < 8; j++)
        childBoundBox += child[i]->TransformFrom(childBox.Corner(j));
    }
  }
  return childBoundBox;
}
const Box& Node::GetChildBoundBox() const { return childBoundBox; }
// Object management
const Object* Node::GetNodeObj() const { return obj; }
Object* Node::GetNodeObj() { return obj; }
void Node::SetNodeObj(Object *object) { obj = object; }

// Material management
const Material* Node::GetMaterial() const { return mtl; }
void Node::SetMaterial(Material *material) { mtl = material; }
// Transformations
Ray Node::ToNodeCoords(const Ray &ray) const
{
  Ray r;
  r.p = TransformTo(ray.p);
  r.dir = TransformTo(ray.p + ray.dir) - r.p;
  return r;
}
DiffRay Node::ToNodeCoords(const DiffRay &ray) const
{
  DiffRay r;
  r.c = ToNodeCoords(ray.c);
  r.x = ToNodeCoords(ray.x);
  r.y = ToNodeCoords(ray.y);
  return r;
}
void Node::FromNodeCoords(HitInfo &hInfo) const
{
  hInfo.p = TransformFrom(hInfo.p);
  hInfo.N = glm::normalize(VectorTransformFrom(hInfo.N));
}
void Node::FromNodeCoords(DiffHitInfo &hInfo) const
{
  FromNodeCoords(hInfo.c);
  hInfo.x.p = TransformFrom(hInfo.x.p);
  hInfo.x.N = glm::normalize(VectorTransformFrom(hInfo.x.N));
  hInfo.y.p = TransformFrom(hInfo.y.p);
  hInfo.y.N = glm::normalize(VectorTransformFrom(hInfo.y.N));
}
}