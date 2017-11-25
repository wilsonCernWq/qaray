//------------------------------------------------------------------------------
///
/// \file       tile.h
/// \author     Qi WU
///
/// \brief This file defines the tile class
///
//------------------------------------------------------------------------------

#pragma once

#include "math/math.h"

#define TILE_SIZE 64

namespace qaray {
//! \brief Tile class is similar to OSPRay tile, which defines a block of
//! images rendered by a single thread. We separate r, g, b, a channels into
//! four different array in order to stream data from SIMD data type.
class Tile {
  int tile_id = 0;
  float r[TILE_SIZE * TILE_SIZE];
  float g[TILE_SIZE * TILE_SIZE];
  float b[TILE_SIZE * TILE_SIZE];
  float a[TILE_SIZE * TILE_SIZE];
  float z[TILE_SIZE * TILE_SIZE];
};
};
