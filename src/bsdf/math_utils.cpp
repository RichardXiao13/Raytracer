#include "math_utils.h"

Vector3D transformToWorld(float x, float y, float z, const Vector3D &normal) {
  // Find an axis that is not parallel to normal
  Vector3D majorAxis;
  if (abs(normal.x) < M_1_SQRT_3) {
    majorAxis = Vector3D(1, 0, 0);
  } else if (abs(normal.y) < M_1_SQRT_3) {
    majorAxis = Vector3D(0, 1, 0);
  } else {
    majorAxis = Vector3D(0, 0, 1);
  }

  // Use majorAxis to create a coordinate system relative to world space
  Vector3D u = normalized(cross(normal, majorAxis));
  Vector3D v = cross(normal, u);
  Vector3D w = normal;

  // Transform from local coordinates to world coordinates
  return u * x + v * y + w * z;
}

Vector3D sphericalToUV(const Vector3D &point, std::shared_ptr<PNG> textureMap) {
  Vector3D normalizedPoint = normalized(point);
  float cosPhi = normalizedPoint.z;
  float phi = std::acos(cosPhi);
  float theta = M_PI + std::atan2(normalizedPoint.y, normalizedPoint.x);
  float width = textureMap->width();
  float height = textureMap->height();
  float y = (height - 1) - clamp(phi * 0.5 * M_1_PI * height, 0, height - 1);
  float x = clamp(theta * 0.5 * M_1_PI * width, 0, width - 1);
  return Vector3D(x, y, 0.0f);
}

Vector3D faceForward(const Vector3D &w, const Vector3D &n) {
  return dot(w, n) < 0.0f ? -n : n;
}

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