#pragma once

#include "../vector/vector3d.h"

class Camera {
public:
  Camera(
    const Vector3D &eye     = Vector3D(0, 0, 0),
    const Vector3D &forward = Vector3D(0, 0, -1),
    const Vector3D &right   = Vector3D(1, 0, 0),
    const Vector3D &up      = Vector3D(0, 1, 0))
    : eye(eye), forward(forward), right(right), up(up) {};
  
  void setEye(const Vector3D& e);
  void setForward(const Vector3D& f);
  void setUp(const Vector3D& u);

  Vector3D eye;     // POINT; NOT NORMALIZED
  Vector3D forward; // NOT NORMALIZED; LONGER VECTOR FOR NARROWER FIELD OF VIEW
  Vector3D right;   // NORMALIZED
  Vector3D up;      // NORMALIZED
};
