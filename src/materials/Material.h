#pragma once

#include "../macros.h"
#include "../vector3d.h"

class Material {
public:
  Material(
    Vector3D shine,
    Vector3D transparency,
    float indexOfRefraction,
    float roughness
  ) :
    shine(shine),
    transparency(transparency),
    indexOfRefraction(indexOfRefraction),
    roughness(roughness),
    roughnessDistribution(0, roughness) {};
  
  float getPerturbation(std::mt19937 &rng) {
    return roughnessDistribution(rng);
  }

  Vector3D getPerturbation3D(std::mt19937 &rng) {
    return Vector3D(roughnessDistribution(rng), roughnessDistribution(rng), roughnessDistribution(rng));
  }

  Vector3D shine;
  Vector3D transparency;
  float indexOfRefraction;
  float roughness;
  std::normal_distribution<float> roughnessDistribution;
};
