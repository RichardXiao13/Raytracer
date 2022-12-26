#pragma once

#include <cmath>

#include "vector3d.h"

/**
 * reflect - reflect a ray using the incident direction and surface normal.
 * 
 * incident - normalized vector
 * normal - normalized vector
*/
Vector3D reflect(const Vector3D& incident, const Vector3D& normal);

/**
 * refract - refract a ray using the incident direction, surface normal, and index of refraction.
 * 
 * incident - normalized vector
 * normal - normalized vector
 * ior - index of refraction
 * point - intersection point
 * bias - small shift to the intersection point
*/
Vector3D refract(const Vector3D& incident, Vector3D& normal, float ior, Vector3D &point, float bias);