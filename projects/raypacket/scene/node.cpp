//------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.1
/// \date       August 29, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///
/// \file       node.cpp
/// \author     Qi WU
///
/// \brief Modified Node class for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "node.h"

namespace qw {

  Node::Node() : child(NULL), numChild(0), obj(NULL), mtl(NULL) {}
  Node::~Node() { DeleteAllChildNodes(); }

  // Initialize the node deleting all child nodes
  void Node::Init() {
    DeleteAllChildNodes();
    obj=NULL; mtl=NULL; SetName(NULL); 
    InitTransform(); 
  }

  // Hierarchy management
  int  Node::GetNumChild() const { return numChild; }
  void Node::SetNumChild(const int n, const int keepOld = false)
  {
    if ( n < 0 ) n=0;	// just to be sure
    Node **nc = NULL;	// new child pointer
    if ( n > 0 ) nc = new Node*[n];
    for ( int i=0; i<n; i++ ) nc[i] = NULL;
    if ( keepOld ) {
      int sn = min(n,numChild);
      for ( int i=0; i<sn; i++ ) nc[i] = child[i];
    }
    if ( child ) delete [] child;
    child = nc;
    numChild = n;
  }
  const Node* Node::GetChild(const int i) const { return child[i]; }
  Node*       Node::GetChild(const int i)       { return child[i]; }
  void        Node::SetChild(const int i, Node *node) { child[i] = node; }
  void Node::AppendChild(Node *node)
  { 
    SetNumChild(numChild+1,true); 
    SetChild(numChild-1,node); 
  }
  void Node::RemoveChild(const int i)
  {
    for ( int j=i; j<numChild-1; j++) { 
      child[j]=child[j+1]; 
    }
    SetNumChild(numChild-1); 
  }
  void Node::DeleteAllChildNodes()
  { 
    for ( int i=0; i<numChild; i++ ) { 
      child[i]->DeleteAllChildNodes(); 
      delete child[i]; 
    } 
    SetNumChild(0); 
  }

  // Object management
  const Object* Node::GetNodeObj() const { return obj; }
  Object*       Node::GetNodeObj()       { return obj; }
  void          Node::SetNodeObj(Object *object) { obj=object; }

  // Material management
  const Material* Node::GetMaterial() const { return mtl; }
  Material*       Node::GetMaterial()       { return mtl; }
  void            Node::SetMaterial(Material *material) { mtl=material; }

  // Transformations
  RayPacket Node::ToNodeCoords(const RayPacket &ray) const
  {
    RayPacket r;
    r.ori = TransformTo(ray.ori);
    r.dir = TransformTo(ray.ori + ray.dir) - r.ori;
    return r;
  }
  void Node::FromNodeCoords( HitInfo &hInfo ) const
  {
    hInfo.p = TransformFrom(hInfo.p);
    hInfo.N = VectorTransformFrom(hInfo.N).GetNormalized();
  }
  
};
