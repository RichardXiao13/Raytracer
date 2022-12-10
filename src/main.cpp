#include <iostream>

#include "lodepng.h"
#include "vector3d.h"

int main(int argc, char **argv) {
  Vector3D x(1, 0, 0);
  Vector3D y(0, 1, 0);
  double z = dot(x, y);
  std::cout << z << std::endl;
  return 0;
}
