#include "macros.h"
#include "vector3d.h"

Vector3D::Vector3D(float x1, float y1, float z1) : x(x1), y(y1), z(z1) {};

float& Vector3D::operator[](int i) {
  assert(i >= 0 && i <= 2);
  if (i == 0) return x;
  if (i == 1) return y;
  return z;
}
float Vector3D::operator[](int i) const {
  assert(i >= 0 && i <= 2);
  if (i == 0) return x;
  if (i == 1) return y;
  return z;
}

Vector3D Vector3D::operator-(const Vector3D& other) const {
  return Vector3D(x - other.x, y - other.y, z - other.z);
}

Vector3D Vector3D::operator+(const Vector3D& other) const {
  return Vector3D(x + other.x, y + other.y, z + other.z);
}

Vector3D &Vector3D::operator+=(const Vector3D& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

Vector3D Vector3D::operator*(const Vector3D& other) const {
  return Vector3D(x * other.x, y * other.y, z * other.z);
}

Vector3D &Vector3D::operator*=(float scalar) {
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

Vector3D operator*(float scalar, const Vector3D& v) {
  return Vector3D(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3D operator*(const Vector3D& v, float scalar) {
  return Vector3D(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3D operator/(float scalar, const Vector3D& v) {
  return Vector3D(scalar / v.x, scalar / v.y, scalar / v.z);
}

Vector3D operator/(const Vector3D& v, float scalar) {
  return Vector3D(v.x / scalar, v.y / scalar, v.z / scalar);
}

Vector3D operator-(float scalar, const Vector3D& v) {
  return Vector3D(scalar - v.x, scalar - v.y, scalar - v.z);
}

Vector3D operator-(const Vector3D& v, float scalar) {
  return Vector3D(v.x - scalar, v.y - scalar, v.z - scalar);
}

float magnitude(const Vector3D& v) {
  return std::sqrt(dot(v, v));
}

Vector3D normalized(const Vector3D& v) {
  float invMag = 1.0f / magnitude(v);
  return Vector3D(v.x * invMag, v.y * invMag, v.z * invMag);
}

float dot(const Vector3D& v1, const Vector3D& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3D cross(const Vector3D& v1, const Vector3D& v2) {
  float i = v1.y * v2.z - v2.y * v1.z;
  float j = -1 * (v1.x * v2.z - v2.x * v1.z);
  float k = v1.x * v2.y - v2.x * v1.y;

  return Vector3D(i, j, k);
}

bool isZero(const Vector3D& v) {
  return v.x + v.y + v.z == 0.0f;
}

float determinant(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3) {
  return v1.x * (v2.y * v3.z - v2.z * v3.y) - v2.x * (v1.y * v3.z - v1.z * v3.y) + v3.x * (v2.z * v1.y - v2.y * v1.z);
}

std::ostream& operator<<(std::ostream& out, const Vector3D& v) {
  out << '<' << v.x << ", " << v.y << ", " << v.z << '>';
  return out;
}
