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
  Vector3D operator-() const;

  friend Vector3D operator*(float scalar, const Vector3D& v);
  friend Vector3D operator*(const Vector3D& v, float scalar);
  friend Vector3D operator/(float scalar, const Vector3D& v);
  friend Vector3D operator/(const Vector3D& v, float scalar);
  friend Vector3D operator-(float scalar, const Vector3D& v);
  friend Vector3D operator-(const Vector3D& v, float scalar);
  friend float magnitude(const Vector3D& v);
  friend Vector3D normalized(const Vector3D& v);
  friend float dot(const Vector3D& v1, const Vector3D& v2);
  friend float clipDot(const Vector3D& v1, const Vector3D& v2);
  
  friend float cosineTheta(const Vector3D& v1, const Vector3D& v2);
  friend float sineTheta(const Vector3D& v1, const Vector3D& v2);
  friend float tangentTheta(const Vector3D& v1, const Vector3D& v2);
  friend float cosinePhi(const Vector3D& v, const Vector3D& n);
  friend float sinePhi(const Vector3D& v, const Vector3D& n);
  friend float tangentPhi(const Vector3D& v, const Vector3D& n);
  
  friend Vector3D cross(const Vector3D& v1, const Vector3D& v2);
  friend bool isZero(const Vector3D& v);
  friend float determinant(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);
  friend float maxDimension(const Vector3D &v);

  float x;
  float y;
  float z;
};

std::ostream& operator<<(std::ostream& out, const Vector3D& v);
