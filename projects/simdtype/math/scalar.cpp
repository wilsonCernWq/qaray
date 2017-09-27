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
namespace qw {
  template class vScalar<simdpp::float32<PACKET_SIZE>, float>;
};
