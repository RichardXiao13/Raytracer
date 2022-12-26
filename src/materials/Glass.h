#pragma once
#include "Material.h"

class Glass : public Material {
public:
  Glass(
    Vector3D shine=Vector3D(1,1,1),
    Vector3D transparency=Vector3D(1,1,1),
    float indexOfRefraction=1.5,
    float roughness=0
  ) : Material(shine, transparency, indexOfRefraction, roughness) {};
};
