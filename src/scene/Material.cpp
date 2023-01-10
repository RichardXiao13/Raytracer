#include "Material.h"


Material::Material()
  : Kd(1.0f), Ks(0.0f), eta(1.0f),
    Kr(1.0f), Kt(0.0f), Ka(0.0f),
    roughness(0.0f),
    type(MaterialType::Dialectric)
{
  bsdf.addBDF(new LambertianReflection(Kd));
};

Material::Material(float Kd, float Ks, float eta, float Kr, float Kt, float Ka, float roughness, MaterialType type)
  : Kd(Kd), Ks(Ks), eta(eta),
    Kr(Kr), Kt(Kt), Ka(Ka),
    roughness(roughness),
    type(type)
{
  BDF *specular = nullptr;

  if (type == MaterialType::Metal) {
    specular = new MicrofacetReflection(Kr, new TrowbridgeReitzDistribution(roughness), new FresnelConductor(1.0f, eta, Ka));
  } else if (type == MaterialType::Mirror) {
    specular = new SpecularReflection(Kr, new FresnelDielectric(1.0f, eta));
  } else if (type == MaterialType::Plastic) {
    // bsdf.addBDF(new LambertianReflection(Kd));
    bsdf.addBDF(new OrenNayarReflection(Kd, 20));
    specular = new MicrofacetReflection(Kr, new TrowbridgeReitzDistribution(roughness), new FresnelDielectric(1.0f, eta));
  } else if (type == MaterialType::Glass) {
    specular = new FresnelSpecular(Kr, Kt, 1.0f, eta);
  }

  if (specular != nullptr) {
    bsdf.addBDF(specular);
  }
};