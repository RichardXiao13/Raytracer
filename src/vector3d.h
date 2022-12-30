#pragma once

#include "macros.h"

class Vector3D {
public:
  Vector3D() {};
  Vector3D(float x1, float y1, float z1);

  float& operator[](int i);
  float operator[](int i) const;
  Vector3D operator-(const Vector3D& other) const;
  Vector3D operator+(const Vector3D& other) const;
  Vector3D &operator+=(const Vector3D& other);
  Vector3D operator*(const Vector3D& other) const;
  Vector3D &operator*=(float scalar);

  friend Vector3D operator*(float scalar, const Vector3D& v);
  friend Vector3D operator*(const Vector3D& v, float scalar);
  friend Vector3D operator/(float scalar, const Vector3D& v);
  friend Vector3D operator/(const Vector3D& v, float scalar);
  friend Vector3D operator-(float scalar, const Vector3D& v);
  friend Vector3D operator-(const Vector3D& v, float scalar);
  friend float magnitude(const Vector3D& v);
  friend Vector3D normalized(const Vector3D& v);
  friend float dot(const Vector3D& v1, const Vector3D& v2);
  friend Vector3D cross(const Vector3D& v1, const Vector3D& v2);
  friend bool isZero(const Vector3D& v);

  float x;
  float y;
  float z;
};

std::ostream& operator<<(std::ostream& out, const Vector3D& v);
