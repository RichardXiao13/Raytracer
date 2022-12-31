#include "math_utils.h"
#include "vector3d.h"

Vector3D reflect(const Vector3D& incident, const Vector3D& normal) {
  return incident - 2.0f * dot(normal, incident) * normal;
}

// refract shouldn't be called if there is total internal reflection given by fresnel
Vector3D refract(const Vector3D& incident, Vector3D& normal, float eta) {
  float enteringCosine = dot(normal, incident);
  if (enteringCosine < 0) {
    enteringCosine = -enteringCosine;
    eta = 1.0f/eta;
  } else {
    normal *= -1.0f;
  }
  float k = 1.0 - eta * eta * (1.0 - enteringCosine * enteringCosine);
  return eta * incident + (eta * enteringCosine - sqrt(k)) * normal;
}

float fresnel(const Vector3D &incident, const Vector3D &normal, const float eta) { 
  // incident and normal need to be normalized
  float cosi = dot(incident, normal); 
  float etai = 1, etat = eta; 
  // Compute sini using Snell's law
  float sint = etai / etat * sqrt(std::max(0.f, 1 - cosi * cosi)); 
  // Total internal reflection
  if (sint >= 1) { 
    return 1.0f;
  } 
  float cost = sqrt(std::max(0.f, 1 - sint * sint)); 
  cosi = abs(cosi); 
  float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
  float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
  return (Rs * Rs + Rp * Rp) / 2; 
  // As a consequence of the conservation of energy, transmittance is given by:
  // kt = 1 - kr;
} 