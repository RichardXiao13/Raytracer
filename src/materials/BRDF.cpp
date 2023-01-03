#include "BRDF.h"

Vector3D sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) {
  float rand = rngInfo.distribution(rngInfo.rng);
  float r = std::sqrtf(rand);
  float theta = rngInfo.distribution(rngInfo.rng) * 2.0f * M_PI;

  float x = r * std::cosf(theta);
  float y = r * std::sinf(theta);

  // Project z up to the unit hemisphere
  float z = std::sqrtf(1.0f - x * x - y * y);

  return normalized(transformToWorld(x, y, z, normal));
}

float integrate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) {
  return 2.0f * M_PI * clipDot(inDirection, normal);
}
