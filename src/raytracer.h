#pragma once

#include <vector>
#include <utility>
#include <random>

#include "PNG.h"
#include "vector3d.h"
#include "BVH.h"
#include "Objects.h"

using namespace std;

// A ray has origin 'eye' and direction 'forward' + Sx * 'right' + Sy * 'up'
double getRayScaleX(double x, int w, int h);
double getRayScaleY(double y, int w, int h);

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
  void createBVH();

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

  Vector3D getEye() {
    return eye;
  }

  void setGlobalIllumination(int gi) {
    globalIllumination = gi;
  }

private:
  RGBAColor illuminate(const IntersectionInfo& info);
  RGBAColor raytrace(const Vector3D& origin, const Vector3D& direction, int depth, int giDepth);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);
  void expose(PNG *img);
  PNG *renderDefault();
  PNG *renderFisheye();

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
  uniform_real_distribution<> uniformDistribution = uniform_real_distribution<>(-0.5, 0.5);
  BVH *bvh;
  bool fisheye = false;
  int globalIllumination = 0;
};

void displayRenderProgress(double progress, int barWidth=70);
