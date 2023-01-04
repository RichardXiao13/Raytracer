#pragma once

#include "math_utils.h"
#include "microfacets.h"

class BDF {
public:
  BDF() {};
  virtual float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const = 0;
  virtual float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf) const;
  virtual float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;
};

class SpecularReflection : public BDF {
public:
  SpecularReflection(const float Kr, Fresnel *fresnel)
    : Kr(Kr), fresnel(fresnel) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf) const;
  float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
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
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf) const;
  float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
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
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf) const;
  float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }

private:
  const float Kr;
  const float Kt;
  const float etaI_;
  const float etaT_;
  const FresnelDielectric fresnel;
};

class LambertianReflection : public BDF {
public:
  LambertianReflection(const float Kr)
    : Kr(Kr) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return Kr * M_1_PI;
  }

private:
  const float Kr;
};

class LambertianTransmission : public BDF {
public:
  LambertianTransmission(const float Kt)
    : Kt(Kt) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return Kt * M_1_PI;
  }

private:
  const float Kt;
};

class MicrofacetReflection : public BDF {
public:
  MicrofacetReflection(const float Kr, const MicrofacetDistribution *distribution, const Fresnel *fresnel)
    : Kr(Kr), distribution(distribution), fresnel(fresnel) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;

private:
  const float Kr;
  const MicrofacetDistribution *distribution;
  const Fresnel *fresnel;
};
