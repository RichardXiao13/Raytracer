#pragma once

#include "math_utils.h"

class BDF {
public:
  BDF() {};
  virtual float func(const Vector3D &wo, const Vector3D &wi) const = 0;
  virtual float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const float sample, float *pdf) const = 0;
  virtual float pdf(const Vector3D &wi, const Vector3D &wo) const = 0;
};

class SpecularReflection : public BDF {
public:
  SpecularReflection(const float Kr, Fresnel *fresnel)
    : Kr(Kr), fresnel(fresnel) {};
  float func(const Vector3D &wo, const Vector3D &wi) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const float sample, float *pdf) const;
  float pdf(const Vector3D &wo, const Vector3D &wi) const {
    return 0.0f;
  }

private:
  const float Kr;
  const Fresnel *fresnel;
};

class SpecularTransmission : public BDF {
public:
  SpecularTransmission(const float Kt, const float etaI, const float etaT)
    : Kt(Kt), etaI_(etaI), etaT_(etaT), fresnel(etaI, etaT) {};
  float func(const Vector3D &wo, const Vector3D &wi) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const float sample, float *pdf) const;
  float pdf(const Vector3D &wo, const Vector3D &wi) const {
    return 0.0f;
  }

private:
  const float Kt;
  const float etaI_;
  const float etaT_;
  const FresnelDielectric fresnel;
};

class FresnelSpecular : public BDF {
public:
  FresnelSpecular(const float Kr, const float Kt, const float etaI, const float etaT)
    : Kr(Kr), Kt(Kt), etaI_(etaI), etaT_(etaT), fresnel(etaI, etaT) {};
  float func(const Vector3D &wo, const Vector3D &wi) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, const float sample, float *pdf) const;
  float pdf(const Vector3D &wo, const Vector3D &wi) const {
    return 0.0f;
  }

private:
  const float Kr;
  const float Kt;
  const float etaI_;
  const float etaT_;
  const FresnelDielectric fresnel;
};