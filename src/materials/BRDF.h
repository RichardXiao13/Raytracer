#pragma once

#include "../macros.h"
#include "../vector3d.h"
#include "../math_utils.h"

Vector3D transformToWorld(float x, float y, float z, const Vector3D &normal);

class BRDF {
public:
  virtual Vector3D sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) = 0;
  virtual float pdf(const Vector3D &inDirection, const Vector3D &normal) = 0;
  virtual float evaluate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) = 0;
};

class LambertBRDF : public BRDF {
public:
  LambertBRDF() = default;
  Vector3D sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo);
  float pdf(const Vector3D &inDirection, const Vector3D &normal);
  float evaluate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal);
};
