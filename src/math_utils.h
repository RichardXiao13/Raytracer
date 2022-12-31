#pragma once

#include "macros.h"
#include "vector3d.h"

struct UniformRNGInfo {
  UniformRNGInfo(std::mt19937 &rng, std::uniform_real_distribution<float> &distribution) : rng(rng), distribution(distribution) {};
  std::mt19937 &rng;
  std::uniform_real_distribution<float> &distribution;
};

/**
 * reflect - reflect a ray using the incident direction and surface normal.
 * 
 * incident - normalized vector
 * normal - normalized vector
*/
Vector3D reflect(const Vector3D& incident, const Vector3D& normal);

/**
 * refract - refract a ray using the incident direction, surface normal, and index of refraction.
 * 
 * incident - normalized vector
 * normal - normalized vector
 * ior - index of refraction
 * point - intersection point
 * bias - small shift to the intersection point
*/
Vector3D refract(const Vector3D& incident, Vector3D& normal, float eta);

float fresnel(const Vector3D &incident, const Vector3D &normal, const float eta);