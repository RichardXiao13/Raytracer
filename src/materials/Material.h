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
  ~Material() {
    // delete diffuseBRDF;
    // delete specularBRDF;
  }
  Material() : eta(1.0f), Kr(0.0f), Kd(1.0f), Ks(0.0f), Ka(0.0f), roughness(0.0f),
  type(MaterialType::Dialectric),
  diffuseBRDF(new LambertBRDF()),
  specularBRDF(new CookTorranceGGX(0.0f, 0.0f, 1.0f, &dialectricFresnel)) {};
  Material(float eta, float Kr, float Kd, float Ks, float Ka, float roughness, MaterialType type) :
  eta(eta),
  Kr(Kr),
  Kd(Kd),
  Ks(Ks),
  Ka(Ka),
  roughness(roughness),
  type(type),
  diffuseBRDF(new LambertBRDF()) {
    if (type == MaterialType::Metal) {
      specularBRDF = new CookTorranceGGX(Ka, roughness, eta, &conductorFresnel);
    } else {
      specularBRDF = new CookTorranceGGX(Ka, roughness, eta, &dialectricFresnel);
    }
  };
  
  float eta;
  float Kr;
  float Kd;
  float Ks;
  float Ka;
  float roughness;
  MaterialType type;
  BRDF *diffuseBRDF;
  BRDF *specularBRDF;
};
