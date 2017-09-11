#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "ray.h"

using namespace qw;

int main() { 
  vec3fv p(0.0f);
  vec3fv d(1.0f);
  RayPacket ray(p, d + d);
  vec3fv c = d + p + d;
  ray.Normalize();
  float val[16];
  simdpp::stream(val, ray.dir.x);
  for (int i = 0; i < 16; ++i)
    std::cout << val[i] << std::endl;

  simdpp::float32<PACKET_SIZE>  x, y, z;
  x = simdpp::splat(1.0f);
  y = simdpp::splat(2.0f);
  z = simdpp::splat(3.0f);
  x = x + y * z / y;

  simdpp::stream(val, x);
  for (int i = 0; i < 16; ++i)
    std::cout << val[i] << std::endl;
  return 0;
}
