#include <iostream>

#include "math/math.h"

using namespace qw;

int main() 
{ 
  float val[PACKET_SIZE];

  vfloat x, y, z;
  vfloat arr[10] = { vfloat(20.0f) };

  x = 1.0f;
  y = 2.0f;
  z = y * 3.0f;
  x = x + y * z / y - z;

  x.Stream(val);
  for (int i = 0; i < PACKET_SIZE; ++i)
    std::cout << val[i] << std::endl;

  x = x + 5.0f;
  x = 5.0f + x;
  x = 20.0f / x;

  x.Stream(val);
  for (int i = 0; i < PACKET_SIZE; ++i)
    std::cout << val[i] << std::endl;

  arr[5] = 10.0f;
  arr[5].Stream(val);
  for (int i = 0; i < PACKET_SIZE; ++i)
    std::cout << val[i] << std::endl;

  arr[4].Stream(val); // wrong
  for (int i = 0; i < PACKET_SIZE; ++i)
    std::cout << val[i] << std::endl;


  return 0;
}
