#pragma once

#include <vector>
#include <ostream>

class Vector3D {
public:
  Vector3D() {};
  Vector3D(double x1, double y1, double z1);

  double& operator[](size_t i);
  double operator[](size_t i) const;
  Vector3D operator-(const Vector3D& other) const;
  Vector3D operator+(const Vector3D& other) const;
  Vector3D operator*(const Vector3D& other) const;
  Vector3D &operator*=(double scalar);

  friend Vector3D operator*(double scalar, const Vector3D& v);
  friend Vector3D operator*(const Vector3D& v, double scalar);
  friend Vector3D operator-(double scalar, const Vector3D& v);
  friend Vector3D operator-(const Vector3D& v, double scalar);
  friend double magnitude(const Vector3D& v);
  friend Vector3D normalized(const Vector3D& v);
  friend double dot(const Vector3D& v1, const Vector3D& v2);
  friend Vector3D cross(const Vector3D& v1, const Vector3D& v2);
private:
  double vec[3];
};

std::ostream& operator<<(std::ostream& out, const Vector3D& v);
