///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/13/17.                                             //
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

#ifndef QARAY_TRIBVH_H
#define QARAY_TRIBVH_H

#include <ext/cyBVH.h>
#include <tiny_obj_loader.h>
#include "TriMesh.h"

namespace qaray {

//! TODO: The BVH has a bug somewhere. Need to look at it when there is time
//! Bounding Volume Hierarchy for triangular meshes (TriMesh)

class BVHTriMesh : public cyBVH {
 public:
  //!@name Constructors
  BVHTriMesh() : mesh(nullptr) {}
  explicit BVHTriMesh(const TriMesh *m) { SetMesh(m); }

  //! Sets the mesh pointer and builds the BVH structure.
  void SetMesh(const TriMesh *m,
               unsigned int maxElementsPerNode = CY_BVH_MAX_ELEMENT_COUNT)
  {
    mesh = m;
    Clear();
    Build((unsigned int)mesh->NF(), maxElementsPerNode);
  }

 protected:
  //! Sets box as the i^th element's bounding box.
  void GetElementBounds(unsigned int i, float box[6]) const override;

  //! Returns the center of the i^th element in the given dimension.
  float GetElementCenter(unsigned int i, int dim) const override;

 private:
  const TriMesh *mesh;
};

};
#endif //QARAY_TRIBVH_H
