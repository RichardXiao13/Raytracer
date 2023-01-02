#pragma once

#include "../macros.h"
#include "../vector3d.h"
#include "../math_utils.h"

Vector3D transformToWorld(float x, float y, float z, const Vector3D &normal);

class BRDF {
public:
  virtual ~BRDF() = default;
  virtual Vector3D sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) const = 0;
  virtual float integrate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) const = 0;
};

class LambertBRDF : public BRDF {
public:
  LambertBRDF() = default;
  ~LambertBRDF() = default;
  Vector3D sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) const;
  float integrate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) const;
};

float dialectricFresnel(const Vector3D &outDirection, const Vector3D &normal, float ior, float k);
float conductorFresnel(const Vector3D &outDirection, const Vector3D &normal, float ior, float k);
float partialGeometryGGX(const Vector3D &direction, const Vector3D &normal, const Vector3D &halfVector, float alpha);

class CookTorranceGGX : public BRDF {
public:
  CookTorranceGGX(float Ka, float roughness, float ior, std::function<float (const Vector3D &, const Vector3D &, float, float)> fresnel)
    : Ka(Ka), roughness(roughness), ior(ior), fresnel(fresnel) {};
  ~CookTorranceGGX() = default;
  Vector3D sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) const;
  float integrate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) const;

  float Ka;
  float roughness;
  float ior;
  std::function<float (const Vector3D &, const Vector3D &, float, float)> fresnel;
};
