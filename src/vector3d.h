#pragma once

#include <vector>
#include <ostream>

class Vector3D {
public:
  Vector3D() {};
  Vector3D(double x1, double y1, double z1);

  double& operator[](size_t i);

  friend Vector3D normalized(Vector3D& v);
  friend double dot(Vector3D& v1, Vector3D& v2);
  friend Vector3D cross(Vector3D& v1, Vector3D& v2);
private:
  double vec[3];
};

std::ostream& operator<<(std::ostream& out, Vector3D& v);
