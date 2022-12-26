#pragma once

#include <random>

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
  
  float getPerturbation() {
    return roughnessDistribution(rng);
  }

  Vector3D shine;
  Vector3D transparency;
  float indexOfRefraction;
  float roughness;
  std::mt19937 rng;
  std::normal_distribution<> roughnessDistribution;
};
