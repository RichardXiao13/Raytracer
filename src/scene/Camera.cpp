#include "Camera.h"

void Camera::setEye(const Vector3D& e) {
    eye = e;
  }

void Camera::setForward(const Vector3D& f) {
  forward = f;
  right = normalized(cross(forward, up));
  up = normalized(cross(right, forward));
}

void Camera::setUp(const Vector3D& u) {
  right = normalized(cross(forward, u));
  up = normalized(cross(right, forward));
}
