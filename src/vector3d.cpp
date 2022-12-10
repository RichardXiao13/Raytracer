#include <cmath>
#include "vector3d.h"

using std::sqrt;

Vector3D::Vector3D(double x1, double y1, double z1) {
  vec[0] = x1;
  vec[1] = y1;
  vec[2] = z1;
}

double& Vector3D::operator[](size_t i) {
  assert(i < 3);
  return vec[i];
}

Vector3D normalized(Vector3D& v) {
  double magnitude = sqrt(dot(v, v));
  double normalizedX = v[0] / magnitude;
  double normalizedY = v[1] / magnitude;
  double normalizedZ = v[2] / magnitude;
  
  return Vector3D(normalizedX, normalizedY, normalizedZ);
}

double dot(Vector3D& v1, Vector3D& v2) {
  double summation = 0;
  for (size_t i = 0; i < 3; ++i) {
    summation += v1[i] * v2[i];
  }
  return summation;
}
Vector3D cross(Vector3D& v1, Vector3D& v2) {
  double i = v1[1] * v2[2] - v2[1] * v1[2];
  double j = -1 * (v1[0] * v2[2] - v2[0] * v1[2]);
  double k = v1[0] * v2[1] - v2[0] * v1[1];

  return Vector3D(i, j, k);
}

std::ostream& operator<<(std::ostream& out, Vector3D& v) {
  out << '<' << v[0] << ", " << v[1] << ", " << v[2] << '>' << std::endl;
  return out;
}
