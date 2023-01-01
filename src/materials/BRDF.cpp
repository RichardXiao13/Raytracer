#include "BRDF.h"

Vector3D transformToWorld(float x, float y, float z, const Vector3D &normal) {
  // Find an axis that is not parallel to normal
  Vector3D majorAxis;
  if (abs(normal.x) < M_1_SQRT_3) {
    majorAxis = Vector3D(1, 0, 0);
  } else if (abs(normal.y) < M_1_SQRT_3) {
    majorAxis = Vector3D(0, 1, 0);
  } else {
    majorAxis = Vector3D(0, 0, 1);
  }

  // Use majorAxis to create a coordinate system relative to world space
  Vector3D u = normalized(cross(normal, majorAxis));
  Vector3D v = cross(normal, u);
  Vector3D w = normal;

  // Transform from local coordinates to world coordinates
  return u * x + v * y + w * z;
}

Vector3D LambertBRDF::sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) {
  float rand = rngInfo.distribution(rngInfo.rng);
  float r = std::sqrtf(rand);
  float theta = rngInfo.distribution(rngInfo.rng) * 2.0f * M_PI;

  float x = r * std::cosf(theta);
  float y = r * std::sinf(theta);

  // Project z up to the unit hemisphere
  float z = std::sqrtf(1.0f - x * x - y * y);

  return normalized(transformToWorld(x, y, z, normal));
}

float LambertBRDF::pdf(const Vector3D &inDirection, const Vector3D &normal) {
  return 0.5f * M_1_PI;
}

float LambertBRDF::evaluate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) {
  return std::max(0.0f, dot(inDirection, normal)) * M_1_PI;
}
