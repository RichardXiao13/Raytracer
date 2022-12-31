#pragma once

#include "../macros.h"
#include "../vector3d.h"
#include "BRDF.h"

enum class MaterialType {
  Dialectric,
  Metal
};

class Material {
public:
  Material() : eta(1.0f), Kr(0.0f), Kd(1.0f), Ks(0.0f), Ka(0.0f), roughness(0.0f),
  type(MaterialType::Dialectric), brdf(new LambertBRDF()) {};
  Material(float eta, float Kr, float Kd, float Ks, float Ka, float roughness, MaterialType type) :
    eta(eta),
    Kr(Kr),
    Kd(Kd),
    Ks(Ks),
    Ka(Ka),
    roughness(roughness),
    n(1.0f/roughness),
    type(type),
    brdf(new LambertBRDF()) {};
  
  float eta;
  float Kr;
  float Kd;
  float Ks;
  float Ka;
  float roughness;
  float n;
  MaterialType type;
  BRDF *brdf;
};
