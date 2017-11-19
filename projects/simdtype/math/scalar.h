//------------------------------------------------------------------------------
///
/// \file       scalar.h
/// \author     Qi WU
///
/// \brief Math library used in this project
///
//------------------------------------------------------------------------------

#pragma once

#include "types.h"

#ifndef USE_LIBSIMDPP
# error "Ray-packet project requires libsimdpp"
#else
# include <simdpp/simd.h>
#endif

#include <type_traits>

//------------------------------------------------------------------------------
namespace qw
{

  //! TCore is the SIMD type in libsimdpp library
  //! TBase is the corresponding built-in scalar type
  template<typename TCore, typename TBase>
  class vScalar
  {

    static_assert(std::is_scalar<TBase>::value, "TBase must be a scalar");

    friend vScalar operator+ (const TBase a, const vScalar &b)
    {
      return vScalar(simdpp::splat<TCore>(a) + b.core);
    }

    friend vScalar operator- (const TBase a, const vScalar &b)
    {
      return vScalar(simdpp::splat<TCore>(a) - b.core);
    }

    friend vScalar operator* (const TBase a, const vScalar &b)
    {
      return vScalar(simdpp::splat<TCore>(a) * b.core);
    }

    friend vScalar operator/ (const TBase a, const vScalar &b)
    {
      return vScalar(simdpp::splat<TCore>(a) / b.core);
    }

  private:
    TCore core;
    TBase *ptr = nullptr;

  private:
    /* constructor & destructor */
    vScalar (const TCore &x) : core(x) {};

    /* assignment operator */
    vScalar &operator= (const TCore &x)
    {
      core = x;
      return *this;
    };

    // Basic operators with TCore
    vScalar operator+ (const TCore &x) const { return vScalar(core + x); };

    vScalar operator- (const TCore &x) const { return vScalar(core - x); };

    vScalar operator* (const TCore &x) const { return vScalar(core * x); };

    vScalar operator/ (const TCore &x) const { return vScalar(core / x); };

  public:
    ~vScalar () = default;

    vScalar () = default;

    vScalar (const vScalar &x) : core(x.core) {};

    vScalar (const TBase x) : core(simdpp::splat(x)) {};

    /* assignment operator */
    vScalar &operator= (const vScalar &x)
    {
      core = x.core;
      return *this;
    };

    vScalar &operator= (const TBase x) { core = simdpp::splat(x); };

    /* comparison operators */
    //bool operator== (HalfFloat other) const;
    //bool operator!= (HalfFloat other) const;

    vScalar operator+ () const { return vScalar(core); };

    vScalar operator- () const
    {
      return vScalar(simdpp::make_float<TCore>(0.0) - core);
    };

    vScalar &operator++ ()
    {
      core = core + 1.f;
      return *this;
    };

    vScalar operator++ (int)
    {
      vScalar ret(*this + 1.f);
      core = core + 1.f;
      return ret;
    };

    vScalar &operator-- ()
    {
      core = core - 1.f;
      return *this;
    };

    vScalar operator-- (int)
    {
      vScalar ret(*this - 1.f);
      core = core - 1.f;
      return ret;
    };

    // Basic operators with vScalar
    vScalar operator+ (const vScalar &x) const { return vScalar(core + x.core); };

    vScalar operator- (const vScalar &x) const { return vScalar(core - x.core); };

    vScalar operator* (const vScalar &x) const { return vScalar(core * x.core); };

    vScalar operator/ (const vScalar &x) const { return vScalar(core / x.core); };

    // Basic operators with TBase
    vScalar operator+ (const TBase x) const
    {
      return vScalar(core + simdpp::splat<TCore>(x));
    };

    vScalar operator- (const TBase x) const
    {
      return vScalar(core - simdpp::splat<TCore>(x));
    };

    vScalar operator* (const TBase x) const
    {
      return vScalar(core * simdpp::splat<TCore>(x));
    };

    vScalar operator/ (const TBase x) const
    {
      return vScalar(core / simdpp::splat<TCore>(x));
    };

    void Stream (void *ptr) const { simdpp::stream(ptr, core); }
  };

  typedef vScalar<simdpp::float32 < PACKET_SIZE>, float>
  vfloat;

};

