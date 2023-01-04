#include "microfacets.h"
#include "vector3d.h"

float TrowbridgeReitzDistribution::distribution(const Vector3D &wh, const Vector3D &n) const {
  float tanTheta2 = tangentTheta(wh, n);
  if (std::isinf(tanTheta2))
    return 0;

  float cosTheta4 = cosineTheta(wh, n);
  cosTheta4 = cosTheta4 * cosTheta4 * cosTheta4 * cosTheta4;
  float cosPhi2 = cosinePhi(wh, n);
  cosPhi2 *= cosPhi2;
  float sinPhi2 = 1 - cosPhi2;
  float t = (cosPhi2 / (alphaX * alphaX) + sinPhi2 / (alphaY * alphaY)) * tanTheta2;
  return 1 / (M_PI * alphaX * alphaY * cosTheta4 * (1 + t) * (1 + t));
}

float TrowbridgeReitzDistribution::lambda(const Vector3D &w, const Vector3D &n) const {
  float absTanTheta = std::abs(tangentTheta(w, n));
  if (std::isinf(absTanTheta))
    return 0;

  float cosPhi2 = cosinePhi(w, n);
  cosPhi2 *= cosPhi2;
  float sinPhi2 = sinePhi(w, n);
  sinPhi2 *= sinPhi2;
  float alpha = std::sqrt(cosPhi2 * alphaX * alphaX + sinPhi2 * alphaY * alphaY);

  float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
  return (-1 + std::sqrt(1 + alpha2Tan2Theta)) / 2;
}