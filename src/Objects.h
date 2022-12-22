#pragma once

#include <random>

#include "vector3d.h"
#include "PNG.h"

using namespace std;

class Object;

/**
 * IntersectInfo struct
 * 
 * t - distance to point of intersection.
 * point - point of intersection.
 * normal - normal direction to the point of intersection.
 * obj - pointer to the Object being intersected with.
*/
struct IntersectionInfo {
  double t;
  Vector3D point;
  Vector3D normal;
  Object *obj;
};


class Object {
public:
  virtual ~Object() {};
  virtual IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction) = 0;

  Vector3D aabbMin() {
    return aabbMin_;
  }

  Vector3D aabbMax() {
    return aabbMax_;
  }

  Vector3D centroid() {
    return centroid_;
  }
  RGBAColor color() {
    return color_;
  }

  void setColor(const RGBAColor& color) {
    color_ = color;
  }

  void setShine(Vector3D s) {
    shine_ = s;
  }

  Vector3D &shine() {
    return shine_;
  }

  void setTransparency(Vector3D t) {
    transparency_ = t;
  }

  Vector3D &transparency() {
    return transparency_;
  }

  void setIndexOfRefraction(double n) {
    indexOfRefraction_ = n;
  }

  double indexOfRefraction() {
    return indexOfRefraction_;
  }

  void setRoughness(double s) {
    roughness_ = s;
    roughnessDistribution = normal_distribution<>(0, roughness_);
  }

  double roughness() {
    return roughness_;
  }

  double getPerturbation() {
    return roughnessDistribution(rng_);
  }

  virtual Vector3D sampleRay();

protected:
  RGBAColor color_;
  Vector3D shine_;
  Vector3D transparency_;
  double indexOfRefraction_;
  double roughness_;
  mt19937 rng_;
  normal_distribution<> roughnessDistribution;
  uniform_real_distribution<> sampleDistribution;
  Vector3D aabbMin_;
  Vector3D aabbMax_;
  Vector3D centroid_;
};

class Light {
public:
  Light(double x, double y, double z, const RGBAColor& color) : color_(color), direction_(x, y, z) {};
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
  Bulb(double x, double y, double z, const RGBAColor& color) : center_(x, y, z), color_(color) {};
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
  Sphere(double x1, double y1, double z1, double r1);
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D center;
  double r;
};

class Plane : public Object {
public:
  Plane(double A1, double B1, double C1, double D1);
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
  Vector3D p2;
  Vector3D p3;
  Vector3D normal;
  Vector3D e1;
  Vector3D e2;
};
