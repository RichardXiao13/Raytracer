#pragma once

#include <random>
#include <memory>

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


class Object {
public:
  virtual ~Object() {};
  virtual IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) = 0;

  void setColor(const RGBAColor& c) {
    color = c;
  }

  Vector3D &shine() {
    return material->shine;
  }

  Vector3D &transparency() {
    return material->transparency;
  }

  float indexOfRefraction() {
    return material->indexOfRefraction;
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
};

class Light {
public:
  Light(float x, float y, float z, const RGBAColor& color) : color_(color), direction_(x, y, z) {};
  RGBAColor &color() {
    return color_;
  }
  Vector3D &direction() {
    return direction_;
  }
private:
  RGBAColor color_;
  Vector3D direction_;
};

class Bulb {
public:
  Bulb(float x, float y, float z, const RGBAColor& color) : center_(x, y, z), color_(color) {};
  Vector3D getLightDirection(const Vector3D& point) const;
  RGBAColor &color() {
    return color_;
  }
  Vector3D &center() {
    return center_;
  }

  Vector3D center() const {
    return center_;
  }

private:
  Vector3D center_;
  RGBAColor color_;
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

private:
  Vector3D normal;
  Vector3D point;
};

class Triangle : public Object {
public:
  Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

private:
  Vector3D p1;
  Vector3D normal;
  Vector3D e1;
  Vector3D e2;
};
