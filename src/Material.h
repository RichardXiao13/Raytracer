#pragma once

#include "macros.h"
#include "vector3d.h"
#include "BDF.h"

enum class MaterialType {
  Dialectric,
  Glass,
  Mirror,
  Plastic,
  Metal
};

/**
 * class Material
 * 
 * Interface for storing information about a material
 * 
 * public members:
 *  Kd           - Total diffuse reflection contribution
 *  Ks           - Total specular reflection contribution
 *  eta          - Index of refraction
 *  Kr           - Specular reflection contribution
 *  Kt           - Specular transmission contribution
 *  Ka           - Absorbance
 *  Roughness    - roughness
 *  type         - Material type
 *  diffuseBRDF  - BRDF for diffuse component
 *  specularBRDF - BRDF for specular component
*/
class Material {
public:
  ~Material() {
    // delete diffuseBRDF;
    // delete specularBRDF;
  }
  Material()
    : Kd(1.0f), Ks(0.0f), eta(1.0f),
      Kr(1.0f), Kt(0.0f), Ka(0.0f),
      roughness(0.0f),
      type(MaterialType::Dialectric),
      diffuseBRDF(new LambertianReflection(Kr)),
      specularBRDF(nullptr) {};
  Material(float Kd, float Ks, float eta, float Kr, float Kt, float Ka, float roughness, MaterialType type)
    : Kd(Kd), Ks(Ks), eta(eta),
      Kr(Kr), Kt(Kt), Ka(Ka),
      roughness(roughness),
      type(type),
      diffuseBRDF(new LambertianReflection(Kr)) {
    if (type == MaterialType::Metal) {
      specularBRDF = new MicrofacetReflection(Kr, new TrowbridgeReitzDistribution(roughness), new FresnelConductor(1.0f, eta, Ka));
      // specularBRDF = new SpecularReflection(Kr, new FresnelConductor(1.0f, eta, Ka));
    } else if (type == MaterialType::Mirror) {
      specularBRDF = new SpecularReflection(Kr, new FresnelDielectric(1.0f, eta));
    } else if (type == MaterialType::Plastic) {
      specularBRDF = new MicrofacetReflection(Kr, new TrowbridgeReitzDistribution(roughness), new FresnelDielectric(1.0f, eta));
      // specularBRDF = new SpecularReflection(Kr, new FresnelDielectric(1.0f, eta));
    } else {
      specularBRDF = new FresnelSpecular(Kr, Kt, 1.0f, eta);
    }
  };
  
  float Kd;
  float Ks;
  float eta;
  float Kr;
  float Kt;
  float Ka;
  float roughness;
  MaterialType type;
  BDF *diffuseBRDF;
  BDF *specularBRDF;
};
