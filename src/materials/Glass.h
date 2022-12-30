#pragma once
#include "Material.h"

class Glass : public Material {
public:
  Glass(
    Vector3D shine=Vector3D(0.04,0.04,0.04),
    Vector3D transparency=Vector3D(1,1,1),
    float indexOfRefraction=1.5f,
    float roughness=0.0f
  ) : Material(shine, transparency, indexOfRefraction, roughness) {};
};
