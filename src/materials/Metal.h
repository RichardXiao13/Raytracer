#pragma once

#include "Material.h"

class Metal : public Material {
public:
  Metal( Vector3D shine, Vector3D transparency, float indexOfRefraction, float roughness)
  : Material(
    shine,
    transparency,
    indexOfRefraction,
    roughness
  ) {};
};