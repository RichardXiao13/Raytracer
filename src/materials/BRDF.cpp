#include "BRDF.h"

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

Vector3D LambertBRDF::sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) const {
  float rand = rngInfo.distribution(rngInfo.rng);
  float r = std::sqrtf(rand);
  float theta = rngInfo.distribution(rngInfo.rng) * 2.0f * M_PI;

  float x = r * std::cosf(theta);
  float y = r * std::sinf(theta);

  // Project z up to the unit hemisphere
  float z = std::sqrtf(1.0f - x * x - y * y);

  return normalized(transformToWorld(x, y, z, normal));
}

float LambertBRDF::integrate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) const {
  return 2.0f * M_PI * clipDot(inDirection, normal);
}

float dialectricFresnel(const Vector3D &outDirection, const Vector3D &halfVector, float ior, float k) {
  float enteringCosine = clipDot(outDirection, halfVector);
  float f0 = (1.0f - ior) / (1.0f + ior);
  f0 = f0 * f0;
  return f0 + (1.0f - f0) * std::pow(1.0f - enteringCosine, 5.0f);
}

float conductorFresnel(const Vector3D &outDirection, const Vector3D &halfVector, float ior, float k) {
  float numerator = (ior - 1.0f) * (ior - 1.0f) + 4.0f * ior * std::pow(1.0f - clipDot(outDirection, halfVector), 5.0f) + k * k;
  float denominator = (ior + 1.0f) * (ior + 1.0f) + k * k;
  return numerator / denominator;
}

float GGX_Distribution(const Vector3D &normal, const Vector3D &halfVector, float alpha) {
  float NoH = clipDot(normal, halfVector);
  float alpha2 = alpha * alpha;
  float NoH2 = NoH * NoH;
  float den = NoH2 * alpha2 + (1 - NoH2);
  float chi = (NoH > 0) ? 1 : 0;
  return (chi * alpha2) / ( M_PI * den * den );
}

float partialGeometryGGX(const Vector3D &direction, const Vector3D &normal, const Vector3D &halfVector, float alpha) {
  float VoH2 = clipDot(direction, halfVector);
  float chi = (VoH2 / clipDot(direction, normal)) > 0 ? 1 : 0;
  VoH2 = VoH2 * VoH2;
  float tan2 = ( 1.0f - VoH2 ) / VoH2;
  return (chi * 2.0f) / ( 1.0f + sqrtf( 1.0f + alpha * alpha * tan2 ) );
}

Vector3D CookTorranceGGX::sample(const Vector3D &outDirection, const Vector3D &normal, UniformRNGInfo &rngInfo) const {
  float rand = rngInfo.distribution(rngInfo.rng);
  float r = std::sqrtf(rand);
  float theta = rngInfo.distribution(rngInfo.rng) * 2.0f * M_PI;

  float x = r * std::cosf(theta);
  float y = r * std::sinf(theta);

  // Project z up to the unit hemisphere
  float z = std::sqrtf(1.0f - x * x - y * y);

  return normalized(transformToWorld(x, y, z, normal));
  // float rand = rngInfo.distribution(rngInfo.rng);
  // float theta = atanf(roughness * sqrtf(rand / (1.0f - rand)));
  // float phi = rngInfo.distribution(rngInfo.rng) * 2.0f * M_PI;

  // float x = std::sinf(theta) * std::cosf(phi);
  // float y = std::sinf(theta) * std::sinf(phi);

  // // Project z up to the unit hemisphere
  // float z = std::cosf(theta);

  // return normalized(transformToWorld(x, y, z, normal));
}

float CookTorranceGGX::integrate(const Vector3D &inDirection, const Vector3D &outDirection, const Vector3D &normal) const {
  Vector3D halfVector = normalized(inDirection + outDirection);
  float cost = clipDot(inDirection, normal);
  float sint = sqrtf(1.0f - cost * cost);
  float distributionPart = GGX_Distribution(normal, halfVector, roughness);
  float fresnelPart = fresnel(outDirection, halfVector, ior, Ka);
  float geometryPart = partialGeometryGGX(outDirection, normal, halfVector, roughness) * partialGeometryGGX(inDirection, normal, halfVector, roughness);
  float numerator = distributionPart * fresnelPart * geometryPart * sint;
  float denominator = 4.0f * clipDot(outDirection, normal);
  return numerator / denominator * 2.0f * M_PI;
  // Vector3D halfVector = normalized(inDirection + outDirection);
  // float cost = clipDot(inDirection, normal);
  // float sint = sqrtf(1.0f - cost * cost);
  // float fresnelPart = fresnel(outDirection, halfVector, ior, Ka);
  // float geometryPart = partialGeometryGGX(outDirection, normal, halfVector, roughness) * partialGeometryGGX(inDirection, normal, halfVector, roughness);
  // float numerator = fresnelPart * geometryPart * sint;
  // float denominator = 4.0f * clipDot(outDirection, normal) * clipDot(halfVector, normal);
  // return numerator / denominator;
}
