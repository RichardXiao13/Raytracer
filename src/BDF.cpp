#include "BDF.h"

float SpecularReflection::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const Vector3D &sample, float *pdf) const {
  *wi = reflect(wo, n);
  *pdf = 1.0f;
  float cosThetaI = dot(*wi, n);
  return fresnel->evaluate(cosThetaI) * Kr / std::abs(cosThetaI);
}

float SpecularTransmission::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const Vector3D &sample, float *pdf) const {
  bool entering = dot(wo, n) > 0;
  float etaI = entering ? etaI_ : etaT_;
  float etaT = entering ? etaT_ : etaI_;

  if (refract(wo, n, etaI / etaT, wi) == false)
    return 0.0f;

  *pdf = 1.0f;
  float cosThetaI = dot(*wi, n);
  float Ft = Kt * (1.0f - fresnel.evaluate(cosThetaI));
  Ft *= (etaI * etaI) / (etaT * etaT);
  return Ft / std::abs(cosThetaI);
}

float FresnelSpecular::sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const Vector3D &sample, float *pdf) const {
  float cosThetaI = dot(wo, n);
  float Fr = fresnelDielectric(cosThetaI, etaI_, etaT_);
  if (sample.x < Fr) {
    *wi = reflect(wo, n);
    *pdf = Fr;
    return Fr * Kr / std::abs(cosThetaI);
  } else {
    bool entering = dot(wo, n) > 0;
    float etaI = entering ? etaI_ : etaT_;
    float etaT = entering ? etaT_ : etaI_;

    if (refract(wo, n, etaI / etaT, wi) == false)
      return 0.0f;

    *pdf = 1.0f - Fr;
    float Ft = Kt * (1.0f - Fr);
    Ft *= (etaI * etaI) / (etaT * etaT);
    return Ft / std::abs(dot(*wi, n));
  }
}