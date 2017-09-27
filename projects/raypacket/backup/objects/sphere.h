#pragma once

#include "scene/object.h"

namespace qw {

  class Sphere : public Object
  {
  public:
    virtual bool IntersectRay(const RayPacket&, HitInfo&, int hitSide=HIT_FRONT) const;
    virtual void ViewportDisplay() const;
  };

};
