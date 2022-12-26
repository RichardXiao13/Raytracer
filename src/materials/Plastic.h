#pragma once

#include "Material.h"

class Plastic : public Material {
public:
  Plastic(
    Vector3D shine=Vector3D(0.25f,0.25f,0.25f),
    Vector3D transparency=Vector3D(0,0,0),
    float indexOfRefraction=0.0f,
    float roughness=0.1f
  ) : Material(shine, transparency, indexOfRefraction, roughness) {};
};
