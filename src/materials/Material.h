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
  type(MaterialType::Dialectric), difffuseBRDF(new LambertBRDF()), specularBRDF(new CookTorranceGGX(0.0f, 1.0f, &conductorFresnel)) {};
  Material(float eta, float Kr, float Kd, float Ks, float Ka, float roughness, MaterialType type) :
    eta(eta),
    Kr(Kr),
    Kd(Kd),
    Ks(Ks),
    Ka(Ka),
    roughness(roughness),
    n(1.0f/roughness),
    type(type),
    difffuseBRDF(new LambertBRDF()),
    specularBRDF(new CookTorranceGGX(roughness, eta, &conductorFresnel)) {};
  
  float eta;
  float Kr;
  float Kd;
  float Ks;
  float Ka;
  float roughness;
  float n;
  MaterialType type;
  BRDF *difffuseBRDF;
  BRDF *specularBRDF;
};
