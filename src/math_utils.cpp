#include "math_utils.h"

Vector3D reflect(const Vector3D& incident, const Vector3D& normal) {
  return incident - 2 * dot(normal, incident) * normal;
}

Vector3D refract(const Vector3D& incident, Vector3D& normal, double ior, Vector3D &point, double bias) {
  double enteringCosine = dot(normal, incident);

  // cosine is positive when vectors are in the same direction
  // so normal and incident ray are in the same direction
  // make normal point inside the object
  if (enteringCosine > 0) {
    normal = -1 * normal;
  } else {
    ior = 1.0 / ior;
    enteringCosine = -enteringCosine;
  }
  
  double k = 1.0 - ior * ior * (1.0 - enteringCosine * enteringCosine);

  if (k >= 0) {
    point = point - bias * normal;
    return ior * incident + (ior * enteringCosine - sqrt(k)) * normal;
  } else {
    // total internal reflection
    point = point + bias * normal;
    return reflect(incident, normal);
  }
}