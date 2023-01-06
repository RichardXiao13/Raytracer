#pragma once

#include "vector3d.h"
#include "math_utils.h"

class MicrofacetDistribution {
public:
  virtual float distribution(const Vector3D &wh, const Vector3D &n) const = 0;
  virtual float lambda(const Vector3D &w, const Vector3D &n) const = 0;
  float partialGeometry(const Vector3D &w, const Vector3D &n) const {
    return 1 / (1 + lambda(w, n));
  }
  float geometry(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 1 / (1 + lambda(wo, n) + lambda(wi, n));
  }
  virtual Vector3D sample_wh(const Vector3D &wo, const Vector3D &n, UniformDistribution &sampler) const = 0;
  float pdf(const Vector3D &wo, const Vector3D &wh, const Vector3D &n) const;
};

class TrowbridgeReitzDistribution : public MicrofacetDistribution {
public:
  TrowbridgeReitzDistribution(const float roughness)
    : alpha(roughnessToAlpha(roughness)) {};
  float distribution(const Vector3D &wh, const Vector3D &n) const;
  float lambda(const Vector3D &w, const Vector3D &n) const;
  Vector3D sample_wh(const Vector3D &wo, const Vector3D &n, UniformDistribution &sampler) const;
  float roughnessToAlpha(const float roughness) const;

private:
  const float alpha;
};