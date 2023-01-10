#pragma once

#include "macros.h"
#include "vector3d.h"
#include "BDF.h"
#include "PNG.h"

enum class MaterialType {
  Dialectric,
  Glass,
  Mirror,
  Plastic,
  Metal
};

const std::unordered_map<std::string, MaterialType> NameToMaterialType = {
  { "dialectric", MaterialType::Dialectric },
  { "glass", MaterialType::Glass },
  { "mirror", MaterialType::Mirror },
  { "plastic", MaterialType::Plastic },
  { "metal", MaterialType::Metal }
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

const std::unordered_map<std::string, std::shared_ptr<Material>> NamedMaterials = {
  { "default", std::make_shared<Material>() },
  { "glass", std::make_shared<Material>(0.0f, 1.0f, 1.5f, 1.0f, 1.0f, 0.0f, 0.0f, MaterialType::Glass) },
  { "plastic", std::make_shared<Material>(0.5f, 0.5f, 1.3f, 1.0f, 0.0f, 0.0f, 0.1f, MaterialType::Plastic) },
  { "copper", std::make_shared<Material>(0.0f, 1.0f, 0.23883f, 0.9553f, 0.0f, 3.415658f, 0.01f, MaterialType::Metal) },
  { "gold", std::make_shared<Material>(0.0f, 1.0f, 0.18104f, 0.99f, 0.0f, 3.068099f, 0.01f, MaterialType::Metal) },
  { "mirror", std::make_shared<Material>(0.0f, 1.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f, MaterialType::Mirror) }
};

// Colors from Wikipedia
const std::unordered_map<std::string, RGBAColor> MaterialColors = {
  { "copper", RGBAColor(0.4793201831f, 0.1714411007f, 0.03310476657f) },
  { "pale copper", RGBAColor(0.7011018919f, 0.2541520943f, 0.1356333297f) },
  { "copper red", RGBAColor(0.5972017884f, 0.152926152f, 0.08228270713f) },
  { "copper penny", RGBAColor(0.4178850708f, 0.1589608351f, 0.1412632911f) },
  { "gold", RGBAColor(1.0f, 0.6795424696330938f, 0.0f) },
  { "metallic gold", RGBAColor(0.6583748172794485f, 0.4286904966139066f, 0.0382043715953465f) }
};
