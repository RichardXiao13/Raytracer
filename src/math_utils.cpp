#include "math_utils.h"
#include "vector3d.h"

float clamp(float val, float low, float high) {
  return std::max(std::min(val, high), low);
}

Vector3D reflect(const Vector3D& incident, const Vector3D& normal) {
  return 2.0f * dot(normal, incident) * normal - incident;
}

bool refract(const Vector3D &incident, const Vector3D &normal, float eta, Vector3D *wt) {
  float cosThetaI = dot(normal, incident);
  float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
  float sin2ThetaT = eta * eta * sin2ThetaI;
  
  if (sin2ThetaT >= 1)
    return false;
  
  float cosThetaT = std::sqrt(1.0f - sin2ThetaT);
  *wt = eta * -incident + (eta * cosThetaI - cosThetaT) * normal;
  return true;
}

float fresnelDielectric(float cosThetaI, float etaI, float etaT) {
  cosThetaI = clamp(cosThetaI, -1, 1);
  // Potentially swap indices of refraction
  bool entering = cosThetaI > 0.0f;
  if (!entering) {
    std::swap(etaI, etaT);
    cosThetaI = std::abs(cosThetaI);
  }

  // Compute cosThetaT using Snell’s law 
  float sinThetaI = std::sqrt(std::max(0.0f, 1 - cosThetaI * cosThetaI));
  float sinThetaT = etaI / etaT * sinThetaI;
  // Handle total internal reflection
  if (sinThetaT >= 1)
    return 1;

  float cosThetaT = std::sqrt(std::max(0.0f, 1 - sinThetaT * sinThetaT));

  float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                ((etaT * cosThetaI) + (etaI * cosThetaT));
  float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                ((etaI * cosThetaI) + (etaT * cosThetaT));
  return (Rparl * Rparl + Rperp * Rperp) / 2;
}

//https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/#more-1921
float fresnelConductor(float cosThetaI, float etaI, float etaT, float k) {
  cosThetaI = clamp(cosThetaI, -1, 1);
  float eta = etaT / etaI;
  float etak = k / etaI;

  float cosThetaI2 = cosThetaI * cosThetaI;
  float sinThetaI2 = 1.0f - cosThetaI2;
  float eta2 = eta * eta;
  float etak2 = etak * etak;

  float t0 = eta2 - etak2 - sinThetaI2;
  float a2plusb2 = std::sqrt(t0 * t0 + 4 * eta2 * etak2);
  float t1 = a2plusb2 + cosThetaI2;
  float a = std::sqrt(0.5f * (a2plusb2 + t0));
  float t2 = 2.0f * a * cosThetaI;
  float Rs= (t1 - t2) / (t1 + t2);

  float t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
  float t4 = 2.0f * a * cosThetaI * sinThetaI2;
  float Rp = Rs * (t3 - t4) / (t3 + t4);
  
  return (Rs + Rp) / 2;
}

float FresnelDielectric::evaluate(float cosThetaI) const {
  return fresnelDielectric(cosThetaI, etaI, etaT);
}

float FresnelConductor::evaluate(float cosThetaI) const {
  return fresnelConductor(std::abs(cosThetaI), etaI, etaT, k);
}