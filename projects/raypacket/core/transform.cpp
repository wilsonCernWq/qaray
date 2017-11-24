///--------------------------------------------------------------------------//
///                                                                          //
/// Created by Qi WU on 11/23/17.                                             //
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

#include "transform.h"

namespace qaray {
Transformation::Transformation() :
    pos(0, 0, 0),
    tm(1.f),
    itm(1.f)
{}
void Transformation::Transform(const Matrix3 &m)
{
  tm = m * tm;
  pos = m * pos;
  itm = glm::inverse(tm);
}
void Transformation::InitTransform()
{
  pos = Point3(0.f);
  tm = Matrix3(1.f);
  itm = Matrix3(1.f);
}
// Multiplies the given vector with the transpose of the given matrix
Point3 Transformation::TransposeMult(const Matrix3 &m, const Point3 &dir)
{
  Point3 d;
  d.x = dot(glm::column(m, 0), dir);
  d.y = dot(glm::column(m, 1), dir);
  d.z = dot(glm::column(m, 2), dir);
  return d;
}
}
