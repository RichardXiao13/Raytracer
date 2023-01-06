#include "macros.h"
#include "vector3d.h"
#include "math_utils.h"

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

Vector3D Vector3D::operator-() const {
  return Vector3D(-x, -y, -z);
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

float cosineTheta(const Vector3D& v1, const Vector3D& v2) {
  return clamp(dot(v1, v2), -1, 1);
}

float sineTheta(const Vector3D& v1, const Vector3D& v2) {
  float cosTheta = cosineTheta(v1, v2);
  return std::sqrt(1 - cosTheta * cosTheta);
}

float tangentTheta(const Vector3D& v1, const Vector3D& v2) {
  float cosTheta = cosineTheta(v1, v2);
  float sinTheta = sineTheta(v1, v2);
  return sinTheta / cosTheta;
}

float cosinePhi(const Vector3D& v, const Vector3D& n) {
  Vector3D vComponent = v - dot(v, n) * n;
  vComponent = (vComponent - vComponent.z) / magnitude(vComponent);
  return vComponent.x;
}

float sinePhi(const Vector3D& v, const Vector3D& n) {
  Vector3D vComponent = v - dot(v, n) * n;
  vComponent = (vComponent - vComponent.z) / magnitude(vComponent);
  return vComponent.y;
}

float tangentPhi(const Vector3D& v, const Vector3D& n) {
  Vector3D vComponent = v - dot(v, n) * n;
  vComponent = (vComponent - vComponent.z) / magnitude(vComponent);
  return vComponent.y / vComponent.x;
}

float clipDot(const Vector3D& v1, const Vector3D& v2) {
  return std::max(0.0f, dot(v1, v2));
}

/**
 * |i     j      k|
 * |v1.x v1.y v1.z|
 * |v2.x v2.y v2.z|
 * 
 * i * (v1.y*v2.z - v2.y*v1.z)
 * -j * (v1.x*v2.z - v2.x*v1.z)
 * k * (v1.x*v2.y - v2.x*v1.y)
*/

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

float maxDimension(const Vector3D &v) {
  return std::max(v.z, std::max(v.x, v.y));
}

std::ostream& operator<<(std::ostream& out, const Vector3D& v) {
  out << '<' << v.x << ", " << v.y << ", " << v.z << '>';
  return out;
}
