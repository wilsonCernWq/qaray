///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 12/14/17.                                             //
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

#include "TriBVH.h"

namespace qaray {
//! Sets box as the i^th element's bounding box.
void BVHTriMesh::GetElementBounds(unsigned int i, float box[6]) const
{
  const TriMesh::TriFace &f = mesh->F(i);
  vec3f p = mesh->V(f.v[0]->vertex_index);
  box[0] = box[3] = p.x;
  box[1] = box[4] = p.y;
  box[2] = box[5] = p.z;
  for (int j = 1; j < 3; j++) { // for each triangle
    vec3f p = mesh->V((*f.v)[j].vertex_index);
    for (int k = 0; k < 3; k++) { // for each dimension
      if (box[k] > p[k]) box[k] = p[k];
      if (box[k + 3] < p[k]) box[k + 3] = p[k];
    }
  }
}

//! Returns the center of the i^th element in the given dimension.
float BVHTriMesh::GetElementCenter(unsigned int i, int dim) const
{
  const TriMesh::TriFace &f = mesh->F(i);
  return
      (mesh->V(f.v[0]->vertex_index)[dim] +
       mesh->V(f.v[1]->vertex_index)[dim] +
       mesh->V(f.v[2]->vertex_index)[dim]) / 3.0f;
}

}
