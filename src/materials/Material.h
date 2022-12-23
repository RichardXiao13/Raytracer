#pragma once

#include <random>

#include "../vector3d.h"

class Material {
public:
  Material(
    Vector3D shine,
    Vector3D transparency,
    double indexOfRefraction,
    double roughness
  ) :
    shine(shine),
    transparency(transparency),
    indexOfRefraction(indexOfRefraction),
    roughness(roughness),
    roughnessDistribution(0, roughness) {};
  
  double getPerturbation() {
    return roughnessDistribution(rng);
  }

  Vector3D shine;
  Vector3D transparency;
  double indexOfRefraction;
  double roughness;
  std::mt19937 rng;
  std::normal_distribution<> roughnessDistribution;
};
