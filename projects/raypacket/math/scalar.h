//------------------------------------------------------------------------------
///
/// \file       scalar.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#pragma once

#include "macro.h"

#include <type_traits>
#include <iostream>

//------------------------------------------------------------------------------

namespace qaray
{

  template<typename TBase, int NBase>
  struct vScalar
  {

  public:
    friend vScalar operator+ (const TBase a, const vScalar &b)
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = b.v[i] + a;
      return r;
    }

    friend vScalar operator- (const TBase a, const vScalar &b)
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = b.v[i] - a;
      return r;
    }

    friend vScalar operator* (const TBase a, const vScalar &b)
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = b.v[i] * a;
      return r;
    }

    friend vScalar operator/ (const TBase a, const vScalar &b)
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = b.v[i] / a;
      return r;
    }

  public:
    TBase v[NBase];

  public:
    ~vScalar () = default;

    vScalar () = default;

    vScalar (const vScalar &x)
    {
      for (int i = 0; i < NBase; ++i) v[i] = x.v[i];
    };

    vScalar (const TBase x)
    {
      for (int i = 0; i < NBase; ++i) v[i] = x;
    };

    // assignment operator
    vScalar &operator= (const vScalar &x)
    {
      if (&x == this) { return *this; }
      for (int i = 0; i < NBase; ++i) v[i] = x.v[i];
      return *this;
    };

    vScalar &operator= (const TBase x)
    {
      for (int i = 0; i < NBase; ++i) v[i] = x;
      return *this;
    };

    // Basic operators with vScalar
    vScalar operator+ (const vScalar &x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] + x.v[i];
      return r;
    };

    vScalar operator- (const vScalar &x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] - x.v[i];
      return r;
    };

    vScalar operator* (const vScalar &x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] * x.v[i];
      return r;
    };

    vScalar operator/ (const vScalar &x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] / x.v[i];
      return r;
    };

    // Basic operators with TBase
    vScalar operator+ (const TBase x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] + x;
      return r;
    };

    vScalar operator- (const TBase x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] - x;
      return r;
    };

    vScalar operator* (const TBase x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] * x;
      return r;
    };

    vScalar operator/ (const TBase x) const
    {
      vScalar r;
      for (int i = 0; i < NBase; ++i) r.v[i] = v[i] / x;
      return r;

    };

    void Print ()
    {
      std::cout << "[";
      for (int i = 0; i < NBase; ++i) std::cout << v[i] << " ";
      std::cout << "]\n";
    }

  };

  typedef vScalar<bool, PACKET_SIZE> vmask;
  typedef vScalar<int, PACKET_SIZE> vint;
  typedef vScalar<float, PACKET_SIZE> vfloat;
  typedef vScalar<double, PACKET_SIZE> vdouble;

};
