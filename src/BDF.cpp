#include "BDF.h"

BDFType operator|(BDFType lhs, BDFType rhs) {
  return static_cast<BDFType>(static_cast<std::underlying_type<BDFType>::type>(lhs) | 
                              static_cast<std::underlying_type<BDFType>::type>(rhs));
}

BDFType operator&(BDFType lhs, BDFType rhs) {
  return static_cast<BDFType>(static_cast<std::underlying_type<BDFType>::type>(lhs) & 
                              static_cast<std::underlying_type<BDFType>::type>(rhs));
}

float BDF::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const {
  // sample unit hemisphere and map it to the normal
  float rand = rngInfo.distribution(rngInfo.rng);
  float r = std::sqrtf(rand);
  float theta = rngInfo.distribution(rngInfo.rng) * 2.0f * M_PI;

  float x = r * std::cosf(theta);
  float y = r * std::sinf(theta);

  // Project z up to the unit hemisphere
  float z = std::sqrtf(1.0f - x * x - y * y);

  *wi = normalized(transformToWorld(x, y, z, n));
  *pdf = this->pdf(wo, *wi, n);
  *type = BDFType::DIFFUSE;
  return func(wo, *wi, n);
}

float BDF::pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
  // Check if wo and wi are in the same direction before returning pdf
  return (dot(wo, wi) > 0) ? std::abs(dot(wi, n)) * M_1_PI : 0;
}

float SpecularReflection::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const {
  *wi = reflect(wo, n);
  *pdf = 1.0f;
  *type = BDFType::PERFECT_SPECULAR;
  float cosThetaI = dot(*wi, n);
  return fresnel->evaluate(cosThetaI) * Kr / std::abs(cosThetaI);
}

float SpecularTransmission::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const {
  bool entering = dot(wo, n) > 0;
  float etaI = entering ? etaI_ : etaT_;
  float etaT = entering ? etaT_ : etaI_;

  if (refract(wo, faceForward(wo, n), etaI / etaT, wi) == false)
    return 0.0f;

  *pdf = 1.0f;
  *type = BDFType::PERFECT_SPECULAR;
  float cosThetaI = dot(*wi, n);
  float Ft = Kt * (1.0f - fresnel.evaluate(cosThetaI));
  Ft *= (etaI * etaI) / (etaT * etaT);
  return Ft / std::abs(cosThetaI);
}

float FresnelSpecular::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const {
  float sample = rngInfo.distribution(rngInfo.rng);
  float cosThetaI = dot(wo, n);
  *type = BDFType::PERFECT_SPECULAR;

  float Fr = fresnelDielectric(cosThetaI, etaI_, etaT_);
  if (sample < Fr) {
    *wi = reflect(wo, n);
    *pdf = Fr;
    return Fr * Kr / std::abs(cosThetaI);
  } else {
    bool entering = cosThetaI > 0;
    float etaI = entering ? etaI_ : etaT_;
    float etaT = entering ? etaT_ : etaI_;

    if (refract(wo, faceForward(wo, n), etaI / etaT, wi) == false)
      return 0.0f;
      
    *pdf = 1.0f - Fr;
    float Ft = Kt * (1.0f - Fr);
    Ft *= (etaI * etaI) / (etaT * etaT);
    return Ft / std::abs(dot(*wi, n));
  }
}

float MicrofacetReflection::func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
  float cosThetaO = std::abs(cosineTheta(wo, n));
  float cosThetaI = std::abs(cosineTheta(wi, n));
  Vector3D wh = wi + wo;
  if (cosThetaI == 0 || cosThetaO == 0)
    return 0;
  if (wh.x == 0 && wh.y == 0 && wh.z == 0)
    return 0;

  wh = normalized(wh);
  float F = fresnel->evaluate(dot(wi, wh));
  return Kr * distribution->distribution(wh, n) * distribution->geometry(wo, wi, n) * F / (4 * cosThetaI * cosThetaO);
}

float MicrofacetReflection::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const {
  Vector3D wh = distribution->sample_wh(wo, n, rngInfo);
  *wi = reflect(wo, wh);
  // if (dot(wo, *wi) < 0)
  //   return 0;

  *pdf = distribution->pdf(wo, wh, n) / (4 * std::abs(dot(wo, wh)));
  *type = BDFType::REFLECTION;
  return func(wo, *wi, n);
}

float MicrofacetReflection::pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
  if (dot(wo, wi) < 0)
    return 0;
  Vector3D wh = normalized(wo + wi);
  return distribution->pdf(wo, wh, n) / (4 * std::abs(dot(wo, wh)));
}

float BSDF::func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
  int numBDFs = bdfs.size();
  float contribution = 0;
  for (int i = 0; i < numBDFs; ++i) {
    contribution += bdfs[i]->func(wo, wi, n);
  }
  return contribution;
}

float BSDF::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const {
  int numBDFs = bdfs.size();
  // somehow there is a segfault here because idx == numBDFs
  // even though it shouldn't happen because random number is between [0, 1)?
  // so use std::min to fix
  float rand = std::floor(rngInfo.distribution(rngInfo.rng) * numBDFs);
  int idx = std::min(numBDFs - 1, static_cast<int>(rand));
  if (idx >= numBDFs)   std::cout << idx << std::endl;
  BDF *bdf = bdfs[idx];
  float contribution = bdf->sampleFunc(wo, wi, n, rngInfo, pdf, type);
  if (*pdf == 0)
    return 0;
  
  if (bdf->type != BDFType::PERFECT_SPECULAR) {
    for (int i = 0; i < numBDFs; ++i) {
      if (bdfs[i] != bdf) {
        *pdf += bdfs[i]->pdf(wo, *wi, n);
        contribution += bdfs[i]->func(wo, *wi, n);
      }
    }
    *pdf /= numBDFs;
  }

  return contribution;
}

float BSDF::pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
  int numBDFs = bdfs.size();
  float pdf = 0;
  for (int i = 0; i < numBDFs; ++i) {
    pdf += bdfs[i]->pdf(wo, wi, n);
  }
  pdf /= numBDFs;
  return pdf;
}
