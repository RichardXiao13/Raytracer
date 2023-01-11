#pragma once

#include "Material.h"
#include "raytracer.h"

#include "../macros.h"
#include "../vector/vector3d.h"
#include "../image/PNG.h"

class Object;
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
  const Object *obj;
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
  Object(const RGBAColor &color, std::shared_ptr<Material> material, std::shared_ptr<PNG> textureMap)
    : color(color), material(material), textureMap(textureMap) {};
  virtual IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) const = 0;
  virtual RGBAColor getColor(const Vector3D &intersectionPoint) const {
    return color;
  }

  RGBAColor color;
  std::shared_ptr<Material> material;
  std::shared_ptr<PNG> textureMap;
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
  virtual RGBAColor emittedLight(const Vector3D &point) const {
    return RGBAColor(0,0,0,0);
  }

  RGBAColor color;
};

class DistantLight : public Light {
public:
  DistantLight(const Vector3D &direction, const RGBAColor& color)
    : Light(color), direction(direction) {};
  ~DistantLight() {};
  bool pointInShadow(const Vector3D &point, const Scene *scene) const;
  RGBAColor intensity(const Vector3D &point, const Vector3D &n) const;

  Vector3D direction;
};

class PointLight : public Light {
public:
  PointLight(const Vector3D &center, const RGBAColor &color)
    : Light(color), center(center) {};
  ~PointLight() {};
  bool pointInShadow(const Vector3D &point, const Scene *scene) const;
  RGBAColor intensity(const Vector3D &point, const Vector3D &n) const;

  Vector3D center;
};

class EnvironmentLight : public Light {
public:
  EnvironmentLight(const Vector3D &center, float radius, const RGBAColor &color)
    : Light(color), center(center), radius(radius), luminanceMap(nullptr) {};
    EnvironmentLight(const Vector3D &center, float radius, float scale, std::shared_ptr<PNG> luminanceMap)
    : Light(RGBAColor()), center(center), radius(radius), scale(scale), luminanceMap(luminanceMap) {};
  ~EnvironmentLight() {};
  bool pointInShadow(const Vector3D &point, const Scene *scene) const;
  RGBAColor intensity(const Vector3D &point, const Vector3D &n) const;
  RGBAColor emittedLight(const Vector3D &point) const;

  Vector3D center;
  float radius;
  float scale;
  std::shared_ptr<PNG> luminanceMap;
};

class Sphere : public Object {
public:
  Sphere(
    const Vector3D &center,
    float r,
    const RGBAColor &color,
    std::shared_ptr<Material> material,
    std::shared_ptr<PNG> textureMap=nullptr
  );
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) const;
  RGBAColor getColor(const Vector3D &intersectionPoint) const;

  Vector3D center;
  float r;
};

class Plane : public Object {
public:
  Plane(
    const Vector3D &normal,
    float D,
    const RGBAColor &color,
    std::shared_ptr<Material> material,
    std::shared_ptr<PNG> textureMap=nullptr
  );
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) const;

  Vector3D normal;
  Vector3D point;
};

class Triangle : public Object {
public:
  Triangle(
    const Vector3D &p1,
    const Vector3D &p2,
    const Vector3D &p3,
    const RGBAColor &color,
    std::shared_ptr<Material> material,
    const Vector3D &t1=Vector3D(),
    const Vector3D &t2=Vector3D(),
    const Vector3D &t3=Vector3D(),
    std::shared_ptr<PNG> textureMap=nullptr
  );
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) const;
  RGBAColor getColor(const Vector3D &intersectionPoint) const;
  void setTextureCoordinates(const Vector3D &tex1, const Vector3D &tex2, const Vector3D &tex3);

  Vector3D p1;
  Vector3D normal;
  Vector3D e1;
  Vector3D e2;

  Vector3D t1;
  Vector3D t2;
  Vector3D t3;
  
  Vector3D n1;
  Vector3D n2;
  Vector3D n3;
};
