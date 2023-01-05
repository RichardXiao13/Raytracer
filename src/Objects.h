#pragma once

#include "macros.h"
#include "vector3d.h"
#include "PNG.h"
#include "Material.h"
#include "raytracer.h"

class Object;
class Material;
struct UniformRNGInfo;
class Scene;

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
  Object(const RGBAColor &color, const std::shared_ptr<Material> &material)
    : color(color), material(material) {};
  virtual IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) = 0;

  void setColor(const RGBAColor& c) {
    color = c;
  }

  RGBAColor color;
  std::shared_ptr<Material> material;
  Vector3D aabbMin;
  Vector3D aabbMax;
  Vector3D centroid;
  ObjectType type;
};

class Light {
public:
  Light(const RGBAColor& color)
    : color(color) {};
  virtual ~Light() {};
  virtual bool pointInShadow(const Vector3D &point, const Scene *scene) const = 0;
  virtual RGBAColor intensity(const Vector3D &point, const Vector3D &n) const = 0;

  RGBAColor color;
};

class DistantLight : public Light {
public:
  DistantLight(float x, float y, float z, const RGBAColor& color)
    : Light(color), direction(x, y, z) {};
  ~DistantLight() {};
  bool pointInShadow(const Vector3D &point, const Scene *scene) const;
  RGBAColor intensity(const Vector3D &point, const Vector3D &n) const;

  Vector3D direction;
};

class Bulb : public Light {
public:
  Bulb(float x, float y, float z, const RGBAColor& color)
    : Light(color), center(x, y, z) {};
  ~Bulb() {};
  bool pointInShadow(const Vector3D &point, const Scene *scene) const;
  RGBAColor intensity(const Vector3D &point, const Vector3D &n) const;

  Vector3D center;
};

class Sphere : public Object {
public:
  Sphere(float x1, float y1, float z1, float r1, const RGBAColor color, const std::shared_ptr<Material> &material);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D center;
  float r;
};

class Plane : public Object {
public:
  Plane(float A1, float B1, float C1, float D1, const RGBAColor color, const std::shared_ptr<Material> &material);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D normal;
  Vector3D point;
};

class Triangle : public Object {
public:
  Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, const RGBAColor color, const std::shared_ptr<Material> &material);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D p1;
  Vector3D normal;
  Vector3D e1;
  Vector3D e2;
  
  Vector3D n1;
  Vector3D n2;
  Vector3D n3;
};
