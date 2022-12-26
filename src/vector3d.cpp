#include <cmath>
#include "vector3d.h"

using std::sqrt;

Vector3D::Vector3D(float x1, float y1, float z1) {
  vec[0] = x1;
  vec[1] = y1;
  vec[2] = z1;
}

float& Vector3D::operator[](size_t i) {
  assert(i < 3);
  return vec[i];
}

float Vector3D::operator[](size_t i) const {
  assert(i < 3);
  return vec[i];
}

Vector3D Vector3D::operator-(const Vector3D& other) const {
  return Vector3D(vec[0] - other.vec[0], vec[1] - other.vec[1], vec[2] - other.vec[2]);
}

Vector3D Vector3D::operator+(const Vector3D& other) const {
  return Vector3D(vec[0] + other.vec[0], vec[1] + other.vec[1], vec[2] + other.vec[2]);
}

Vector3D Vector3D::operator*(const Vector3D& other) const {
  return Vector3D(vec[0] * other.vec[0], vec[1] * other.vec[1], vec[2] * other.vec[2]);
}

Vector3D &Vector3D::operator*=(float scalar) {
  vec[0] *= scalar;
  vec[1] *= scalar;
  vec[2] *= scalar;
  return *this;
}

Vector3D operator*(float scalar, const Vector3D& v) {
  return Vector3D(v.vec[0] * scalar, v.vec[1] * scalar, v.vec[2] * scalar);
}

Vector3D operator*(const Vector3D& v, float scalar) {
  return Vector3D(v.vec[0] * scalar, v.vec[1] * scalar, v.vec[2] * scalar);
}

Vector3D operator-(float scalar, const Vector3D& v) {
  return Vector3D(scalar - v[0], scalar - v[1], scalar - v[2]);
}

Vector3D operator-(const Vector3D& v, float scalar) {
  return Vector3D(v[0] - scalar, v[1] - scalar, v[2] - scalar);
}

float magnitude(const Vector3D& v) {
  return sqrt(dot(v, v));
}

Vector3D normalized(const Vector3D& v) {
  float mag = magnitude(v);
  float normalizedX = v[0] / mag;
  float normalizedY = v[1] / mag;
  float normalizedZ = v[2] / mag;
  
  return Vector3D(normalizedX, normalizedY, normalizedZ);
}

float dot(const Vector3D& v1, const Vector3D& v2) {
  float summation = 0;
  for (size_t i = 0; i < 3; ++i) {
    summation += v1[i] * v2[i];
  }
  return summation;
}
Vector3D cross(const Vector3D& v1, const Vector3D& v2) {
  float i = v1[1] * v2[2] - v2[1] * v1[2];
  float j = -1 * (v1[0] * v2[2] - v2[0] * v1[2]);
  float k = v1[0] * v2[1] - v2[0] * v1[1];

  return Vector3D(i, j, k);
}

std::ostream& operator<<(std::ostream& out, const Vector3D& v) {
  out << '<' << v[0] << ", " << v[1] << ", " << v[2] << '>';
  return out;
}
