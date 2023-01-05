#pragma once

#include "macros.h"
#include "vector3d.h"
#include "BDF.h"

enum class MaterialType {
  Dialectric,
  Glass,
  Mirror,
  Plastic,
  Metal
};

/**
 * class Material
 * 
 * Interface for storing information about a material
 * 
 * public members:
 *  Kd           - Total diffuse reflection contribution
 *  Ks           - Total specular reflection contribution
 *  eta          - Index of refraction
 *  Kr           - Specular reflection contribution
 *  Kt           - Specular transmission contribution
 *  Ka           - Absorbance
 *  Roughness    - roughness
 *  type         - Material type
 *  diffuseBRDF  - BRDF for diffuse component
 *  specularBRDF - BRDF for specular component
*/
class Material {
public:
  Material();
  Material(float Kd, float Ks, float eta, float Kr, float Kt, float Ka, float roughness, MaterialType type);
  
  float Kd;
  float Ks;
  float eta;
  float Kr;
  float Kt;
  float Ka;
  float roughness;
  MaterialType type;
  BSDF bsdf;
};
