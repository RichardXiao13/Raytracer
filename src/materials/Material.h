#pragma once

#include "../macros.h"
#include "../vector3d.h"
#include "../BDF.h"

enum class MaterialType {
  Dialectric,
  Glass,
  Mirror,
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
  diffuseBRDF(nullptr),
  specularBRDF(nullptr) {};
  Material(float eta, float Kr, float Kd, float Ks, float Ka, float roughness, MaterialType type) :
  eta(eta),
  Kr(Kr),
  Kd(Kd),
  Ks(Ks),
  Ka(Ka),
  roughness(roughness),
  type(type),
  diffuseBRDF(nullptr) {
    if (type == MaterialType::Metal) {
      specularBRDF = new SpecularReflection(Kr, new FresnelConductor(1.0f, eta, Ka));
    } else if (type == MaterialType::Mirror) {
      std::cout << "good" << std::endl;
      specularBRDF = new SpecularReflection(Kr, new FresnelDielectric(1.0f, eta));
    } else {
      specularBRDF = new FresnelSpecular(Kr, 1.0f, 1.0f, eta);
    }
  };
  
  float eta;
  float Kr;
  float Kd;
  float Ks;
  float Ka;
  float roughness;
  MaterialType type;
  BDF *diffuseBRDF;
  BDF *specularBRDF;
};
