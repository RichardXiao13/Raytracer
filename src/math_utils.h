#pragma once

#include "macros.h"
#include "vector3d.h"

struct UniformRNGInfo {
  UniformRNGInfo(std::mt19937 &rng, std::uniform_real_distribution<float> &distribution) : rng(rng), distribution(distribution) {};
  std::mt19937 &rng;
  std::uniform_real_distribution<float> &distribution;
};

Vector3D transformToWorld(float x, float y, float z, const Vector3D &normal);

Vector3D faceForward(const Vector3D &wo, const Vector3D &n);

float clamp(float val, float low, float high);

/**
 * reflect - reflect a ray using the incident direction and surface normal.
 * 
 * incident - normalized vector
 * normal - normalized vector
*/
Vector3D reflect(const Vector3D& incident, const Vector3D& normal);

bool refract(const Vector3D &incident, const Vector3D &normal, float eta, Vector3D *wt);

float fresnelDielectric(float cosThetaI, float etaI, float etaT);

float fresnelConductor(float cosThetaI, float etaI, float etaT, float k);

/**
 * Abstract class Fresnel
 * 
 * Interface for FresnelDielectric and FresnelConductor
*/
class Fresnel {
public:
  virtual ~Fresnel() {};
  /**
   * evaluate
   * 
   * returns the amount of reflected light at the given cosine angle between the
   * incident and normal directions.
  */
  virtual float evaluate(float cosThetaI) const = 0;
};

/**
 * class FresnelDielectric
 * 
 * Manages interface for calculating the amount of reflected light between two
 * dielectric materials
 * 
 * private members:
 *  etaI - Index of refraction for the incident media
 *  etaT - Index of refraction for the transmitted media
*/
class FresnelDielectric : public Fresnel {
public:
  FresnelDielectric(const float etaI, const float etaT)
    : etaI(etaI), etaT(etaT) {};
  float evaluate(float cosThetaI) const;

private:
  float etaI;
  float etaT;
};

/**
 * class FresnelConductor
 * 
 * Manages interface for calculating the amount of reflected light between a coductor
 * and a dielectric material.
 * 
 * private members:
 *  etaI - Index of refraction for the incident (conductor) media
 *  etaT - Index of refraction for the transmitted (dialectric) media
 *  k    - Absorption coefficient for the conductor
*/
class FresnelConductor : public Fresnel {
public:
  FresnelConductor(const float etaI, const float etaT, const float k)
    : etaI(etaI), etaT(etaT), k(k) {};
  float evaluate(float cosThetaI) const;

private:
  float etaI;
  float etaT;
  float k;
};