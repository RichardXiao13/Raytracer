#include <iostream>

#include "lodepng.h"
#include "vector3d.h"

static Vector3D eye     (0, 0, 0); // POINT; NOT NORMALIZED
static Vector3D forward(0, 0, -1); // NOT NORMALIZED; LONGER VECTOR FOR NARROWER FIELD OF VIEW
static Vector3D right   (1, 0, 0); // NORMALIZED
static Vector3D up      (0, 1, 0); // NORMALIZED

int main(int argc, char **argv) {
  Vector3D x(1, 0, 0);
  Vector3D y(0, 1, 0);
  double z = dot(x, y);
  std::cout << z << std::endl;
  return 0;
}
