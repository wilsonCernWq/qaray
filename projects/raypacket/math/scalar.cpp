//------------------------------------------------------------------------------
///
/// \file       scalar.cpp
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#include "scalar.h"

//! Explicit instantiation
namespace qaray {

  template class vScalar<bool, PACKET_SIZE>;
  template class vScalar<int, PACKET_SIZE>;
  template class vScalar<float, PACKET_SIZE>;
  template class vScalar<double, PACKET_SIZE>;

};
