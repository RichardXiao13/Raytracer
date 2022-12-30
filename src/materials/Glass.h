#pragma once
#include "Material.h"

class Glass : public Material {
public:
  Glass(
    float indexOfRefraction=1.5f,
    float roughness=0.0f
  ) : Material(indexOfRefraction, roughness, 1.0f) {};
};
