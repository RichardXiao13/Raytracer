#pragma once

#include <string>
#include <vector>
#include <utility>

#include "PNG.h"
#include "vector3d.h"

using namespace std;

class Object;

// A ray has origin 'eye' and direction 'forward' + Sx * 'right' + Sy * 'up'
double getRayScaleX(double x, int w, int h);
double getRayScaleY(double y, int w, int h);

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
  RGBAColor color() {
    return color_;
  }
  void setColor(const RGBAColor& color) {
    color_ = color;
  }

protected:
  RGBAColor color_;
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
  Vector3D getLightDirection(const Vector3D& point);
  RGBAColor &color() {
    return color_;
  }
  Vector3D &center() {
    return center_;
  }

private:
  Vector3D center_;
  RGBAColor color_;
};

class Sphere : public Object {
public:
  Sphere(double x1, double y1, double z1, double r1) : center(x1, y1, z1), r(r1) {};
  ~Sphere() {};
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

  Vector3D center;
  double r;
};

class Plane : public Object {
public:
  Plane(double A1, double B1, double C1, double D1) : A(A1), B(B1), C(C1), D(D1) {};
  ~Plane() {};
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);

private:
  double A;
  double B;
  double C;
  double D;
};
class Scene {
public:
  Scene(int w, int h, const string& file) : width_(w), height_(h), filename_(file) {};
  ~Scene();
  void addObject(Object *obj);
  void addLight(Light *light);
  void addBulb(Bulb *bulb);
  size_t getNumObjects();
  PNG *render(const Vector3D& eye, const Vector3D& forward, const Vector3D& right, const Vector3D& up);
  bool pointInShadow(const Vector3D& origin, const Vector3D& light);

  int width() {
    return width_;
  }

  int height() {
    return height_;
  }

  string filename() {
    return filename_;
  }

  void setEye(const Vector3D& eye) {
    eye_ = eye;
  }

private:
  RGBAColor illuminate(const IntersectionInfo& info);
  RGBAColor raytrace(const Vector3D& origin, const Vector3D& direction);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);

  vector<Object*> objects;
  vector<Light*> lights;
  vector<Bulb*> bulbs;
  int width_;
  int height_;
  string filename_;
  Vector3D eye_;
  double bias_ = 1e-4;
};
