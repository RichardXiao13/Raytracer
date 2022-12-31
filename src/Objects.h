#pragma once

#include "macros.h"
#include "vector3d.h"
#include "PNG.h"
#include "materials/Material.h"
#include "raytracer.h"

class Object;
struct UniformRNGInfo;

/**
 * IntersectInfo struct
 * 
 * t - distance to point of intersection.
 * point - point of intersection.
 * normal - normal direction to the point of intersection.
 * obj - pointer to the Object being intersected with.
*/
struct IntersectionInfo {
  float t;
  Vector3D point;
  Vector3D normal;
  Object *obj;
};

enum class ObjectType {
  Diffuse,
  Reflective,
  Refractive,
  Metal
};

class Object {
public:
  virtual ~Object() {};
  virtual IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) = 0;

  void setColor(const RGBAColor& c) {
    color = c;
  }

  void setMaterial(std::unique_ptr<Material> mat) {
    material = std::move(mat);
  }

  virtual Vector3D sampleRay(UniformRNGInfo &rngInfo);

  RGBAColor color;
  std::unique_ptr<Material> material;
  Vector3D aabbMin;
  Vector3D aabbMax;
  Vector3D centroid;
  ObjectType type;
};

class Light {
public:
  Light(float x, float y, float z, const RGBAColor& color) : color(color), direction(x, y, z) {};

  RGBAColor color;
  Vector3D direction;
};

class Bulb {
public:
  Bulb(float x, float y, float z, const RGBAColor& color) : center(x, y, z), color(color) {};
  Vector3D getLightDirection(const Vector3D& point) const;

  Vector3D center;
  RGBAColor color;
};

class Sphere : public Object {
public:
  Sphere(float x1, float y1, float z1, float r1);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D center;
  float r;
};

class Plane : public Object {
public:
  Plane(float A1, float B1, float C1, float D1);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D normal;
  Vector3D point;
};

class Triangle : public Object {
public:
  Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D p1;
  Vector3D normal;
  Vector3D e1;
  Vector3D e2;
};
