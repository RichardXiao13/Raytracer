#pragma once

#include "math_utils.h"
#include "microfacets.h"

enum class BDFType : char {
  DIFFUSE          = 1 << 0,
  REFLECTION       = 1 << 1,
  TRANSMISSION     = 1 << 2,
  PERFECT_SPECULAR = 1 << 3,
};

BDFType operator|(BDFType lhs, BDFType rhs);
BDFType operator&(BDFType lhs, BDFType rhs);
class BDF {
public:
  BDF(BDFType type) : type(type) {};
  virtual ~BDF() {};
  virtual float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const = 0;
  virtual float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const;
  virtual float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;

  BDFType type;
};

class SpecularReflection : public BDF {
public:
  SpecularReflection(const float Kr, Fresnel *fresnel)
    : BDF(BDFType::REFLECTION | BDFType::PERFECT_SPECULAR), Kr(Kr), fresnel(fresnel) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const;
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
    : BDF(BDFType::TRANSMISSION | BDFType::PERFECT_SPECULAR), Kt(Kt), etaI_(etaI), etaT_(etaT), fresnel(etaI, etaT) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const;
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
    : BDF(BDFType::REFLECTION | BDFType::TRANSMISSION | BDFType::PERFECT_SPECULAR), Kr(Kr), Kt(Kt), etaI_(etaI), etaT_(etaT), fresnel(etaI, etaT) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return 0.0f;
  }
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const;
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
    : BDF(BDFType::REFLECTION | BDFType::DIFFUSE), Kr(Kr) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return Kr * M_1_PI;
  }

private:
  const float Kr;
};

class LambertianTransmission : public BDF {
public:
  LambertianTransmission(const float Kt)
    : BDF(BDFType::TRANSMISSION | BDFType::DIFFUSE), Kt(Kt) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const {
    return Kt * M_1_PI;
  }

private:
  const float Kt;
};

class MicrofacetReflection : public BDF {
public:
  MicrofacetReflection(const float Kr, const MicrofacetDistribution *distribution, const Fresnel *fresnel)
    : BDF(BDFType::REFLECTION), Kr(Kr), distribution(distribution), fresnel(fresnel) {};
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const;
  float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;
  
private:
  const float Kr;
  const MicrofacetDistribution *distribution;
  const Fresnel *fresnel;
};

class BSDF {
public:
  ~BSDF() {
    for (auto it = bdfs.begin(); it != bdfs.end(); ++it) {
      delete *it;
    }
  }
  void addBDF(BDF *bdf) {
    bdfs.push_back(bdf);
  }
  float func(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;
  float sampleFunc(const Vector3D &wo, Vector3D *wi, const Vector3D &n, UniformRNGInfo &rngInfo, float *pdf, BDFType *type) const;
  float pdf(const Vector3D &wo, const Vector3D &wi, const Vector3D &n) const;

private:
  std::vector<BDF*> bdfs;
};
