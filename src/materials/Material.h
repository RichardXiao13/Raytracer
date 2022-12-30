#pragma once

#include "../macros.h"
#include "../vector3d.h"

class Material {
public:
  Material() : eta(1.0f), Kr(0.0f), roughness(0.0f) {};
  Material(float eta, float Kr, float roughness) :
    eta(eta),
    Kr(Kr),
    roughness(roughness),
    roughnessDistribution(0, roughness) {};
  
  float getPerturbation(std::mt19937 &rng) {
    return roughnessDistribution(rng);
  }

  Vector3D getPerturbation3D(std::mt19937 &rng) {
    return Vector3D(roughnessDistribution(rng), roughnessDistribution(rng), roughnessDistribution(rng));
  }

  float eta;
  float Kr;
  float roughness;
  std::normal_distribution<float> roughnessDistribution;
};
