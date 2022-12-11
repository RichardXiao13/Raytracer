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

class Sphere : public Object {
public:
  Sphere(double x1, double y1, double z1, double r1) : center(x1, y1, z1), r(r1) {};
  ~Sphere() {};
  IntersectionInfo intersect(const Vector3D& origin, const Vector3D& direction);
  void setColor(const RGBAColor& c) {
    color_ = c;
  }

  Vector3D center;
  double r;
};
class Scene {
public:
  Scene(int w, int h, const string& file) : width_(w), height_(h), filename_(file) {};
  ~Scene();
  void addObject(Object *obj);
  void addLight(Light *light);
  size_t getNumObjects();
  PNG *render(const Vector3D& eye, const Vector3D& forward, const Vector3D& right, const Vector3D& up);

  int width() {
    return width_;
  }

  int height() {
    return height_;
  }

  string filename() {
    return filename_;
  }

private:
  RGBAColor illuminate(const RGBAColor& objectColor, const Vector3D& surfaceNormal);
  RGBAColor raytrace(const Vector3D& origin, const Vector3D& direction);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);

  vector<Object*> objects;
  vector<Light*> lights;
  int width_;
  int height_;
  string filename_;
};
