#pragma once

#include <string>
#include <vector>
#include <utility>
#include <random>

#include "PNG.h"
#include "vector3d.h"
#include "BVH.h"

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

  double getPerturbation(mt19937& rng) {
    return roughnessDistribution(rng);
  }

protected:
  RGBAColor color_;
  Vector3D shine_;
  Vector3D transparency_;
  double indexOfRefraction_;
  double roughness_;
  normal_distribution<> roughnessDistribution;
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
class Scene {
public:
  Scene(int w, int h, const string& file)
  : width_(w), height_(h), filename_(file), eye(0, 0, 0), forward(0, 0, -1), right(1, 0, 0), up(0, 1, 0) {};
  ~Scene();
  void addObject(Object *obj);
  void addPlane(Plane *plane);
  void addLight(Light *light);
  void addBulb(Bulb *bulb);
  void addPoint(double x, double y, double z);
  Vector3D &getPoint(int i);
  size_t getNumObjects();
  PNG *render(int seed=56);
  bool pointInShadow(const Vector3D& origin, const Vector3D& light);
  bool pointInShadow(const Vector3D& point, const Bulb *bulb);
  void setExposure(double value);
  void setMaxBounces(int d);

  int width() {
    return width_;
  }

  int height() {
    return height_;
  }

  string filename() {
    return filename_;
  }

  void setEye(const Vector3D& e) {
    eye = e;
  }

  void setForward(const Vector3D& f) {
    forward = f;
    right = normalized(cross(forward, up));
    up = normalized(cross(right, forward));
  }

  void setUp(const Vector3D& u) {
    right = normalized(cross(forward, u));
    up = normalized(cross(right, forward));
  }

  void setNumRays(int n) {
    numRays = n;
  }

  void enableFisheye() {
    fisheye = true;
  }

private:
  RGBAColor illuminate(const IntersectionInfo& info);
  RGBAColor raytrace(const Vector3D& origin, const Vector3D& direction, int depth);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);

  vector<Object*> objects;
  vector<Plane*> planes;
  vector<Light*> lights;
  vector<Bulb*> bulbs;
  vector<Vector3D> points;
  int width_;
  int height_;
  string filename_;
  Vector3D eye; // POINT; NOT NORMALIZED
  Vector3D forward; // NOT NORMALIZED; LONGER VECTOR FOR NARROWER FIELD OF VIEW
  Vector3D right; // NORMALIZED
  Vector3D up; // NORMALIZED
  double bias_ = 1e-4;
  double exposure = -1.0;
  int maxBounces = 4;
  int numRays = 1;
  mt19937 rng;
  uniform_real_distribution<> aliasingDistribution = uniform_real_distribution<>(-0.5, 0.5);
  BVH *bvh;
  bool fisheye = false;
};

void displayRenderProgress(double progress, int barWidth=70);
