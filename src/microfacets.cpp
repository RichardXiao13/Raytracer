#include "microfacets.h"
#include "vector3d.h"

float MicrofacetDistribution::pdf(const Vector3D &wo, const Vector3D &wh, const Vector3D &n) const {
  return distribution(wh, n) * std::abs(cosineTheta(wh, n));
}

float TrowbridgeReitzDistribution::distribution(const Vector3D &wh, const Vector3D &n) const {
  float tanTheta2 = tangentTheta(wh, n);
  tanTheta2 *= tanTheta2;
  if (std::isinf(tanTheta2))
    return 0;

  float cosTheta4 = cosineTheta(wh, n);
  cosTheta4 = cosTheta4 * cosTheta4 * cosTheta4 * cosTheta4;
  float cosPhi2 = cosinePhi(wh, n);
  cosPhi2 *= cosPhi2;
  float sinPhi2 = 1 - cosPhi2;
  float alpha2 = alpha * alpha;
  float t = tanTheta2 / alpha2;
  return 1 / (M_PI * alpha2 * cosTheta4 * (1 + t) * (1 + t));
}

float TrowbridgeReitzDistribution::lambda(const Vector3D &w, const Vector3D &n) const {
  float absTanTheta = std::abs(tangentTheta(w, n));
  if (std::isinf(absTanTheta))
    return 0;

  float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
  return (-1 + std::sqrt(1 + alpha2Tan2Theta)) / 2;
}

Vector3D TrowbridgeReitzDistribution::sample_wh(const Vector3D &wo, const Vector3D &n, UniformDistribution &sampler) const {
  float rand = sampler();
  
  float theta = atanf(alpha * sqrtf(rand / (1.0f - rand)));
  float phi = sampler() * 2.0f * M_PI;

  float x = std::sinf(phi) * std::cosf(theta);
  float y = std::sinf(phi) * std::sinf(theta);

  // Project z up to the unit hemisphere
  float z = std::cosf(phi);

  return faceForward(normalized(transformToWorld(x, y, z, n)), n);
}

float TrowbridgeReitzDistribution::roughnessToAlpha(const float roughness) const {
  return std::sqrt(std::min(1e-4f, roughness));
}