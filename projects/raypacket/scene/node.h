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
/// \file       node.h 
/// \author     Qi WU
///
/// \brief Modified Node class for ray-packet project
///
//------------------------------------------------------------------------------

#pragma once

#include "scene_fwd.h"
#include "common/transform.h"
#include "common/item.h"

namespace qw 
{
  class Node : public ItemBase, public Transformation {
  private:
    Node **child;    //!< Child nodes
    size_t numChild; //!< The number of child nodes
    Object   *obj;   //!< Object reference (merely points to the object, but does not own the object)
    Material *mtl;   //!< Material used for shading the object
  public:
    Node();
    virtual ~Node();
    // Initialize the node deleting all child nodes
    void Init();
    // Hierarchy management
    int	 GetNumChild() const;
    void SetNumChild(const int n, const int keepOld = false);
    const Node* GetChild(const int i) const;
    Node*       GetChild(const int i);
    void        SetChild(const int i, Node *node);
    void AppendChild(Node *node);
    void RemoveChild(const int i);
    void DeleteAllChildNodes();
    // Object management
    const Object* GetNodeObj() const;
    Object*       GetNodeObj();
    void          SetNodeObj(Object *object);
    // Material management
    const Material* GetMaterial() const;
    Material*       GetMaterial();
    void            SetMaterial(Material *material);
    // Transformations
    RayPacket ToNodeCoords(const RayPacket &ray) const;
    void FromNodeCoords( HitInfo &hInfo ) const;
  };
};
