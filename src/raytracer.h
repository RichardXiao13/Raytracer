#pragma once

#include <vector>
#include <utility>
#include <random>
#include <functional>

#include "PNG.h"
#include "vector3d.h"
#include "BVH.h"
#include "Objects.h"
#include "SafeQueue.h"
#include "SafeProgressBar.h"

// A ray has origin 'eye' and direction 'forward' + Sx * 'right' + Sy * 'up'
float getRayScaleX(float x, int w, int h);
float getRayScaleY(float y, int w, int h);

struct RenderTask {
  int x;
  int y;
};

struct UniformRNGInfo {
  UniformRNGInfo(std::mt19937 &rng, std::uniform_real_distribution<float> &distribution) : rng(rng), distribution(distribution) {};
  std::mt19937 &rng;
  std::uniform_real_distribution<float> &distribution;
};

class Scene {
public: 
  Scene(int w, int h, const std::string& file)
  : width_(w), height_(h), filename_(file), eye(0, 0, 0), forward(0, 0, -1), right(1, 0, 0), up(0, 1, 0) {};
  void addObject(std::unique_ptr<Object> obj);
  void addPlane(std::unique_ptr<Plane> plane);
  void addLight(std::unique_ptr<Light> light);
  void addBulb(std::unique_ptr<Bulb> bulb);
  size_t getNumObjects();
  PNG *render(int numThreads=4, int seed=56);
  bool pointInShadow(const Vector3D& origin, const Vector3D& light);
  bool pointInShadow(const Vector3D& point, const std::unique_ptr<Bulb>& bulb);
  void setExposure(float value);
  void setMaxBounces(int d);
  void setFilename(const std::string& fname);
  void createBVH(int numThreads);

  int width() {
    return width_;
  }

  int height() {
    return height_;
  }

  std::string filename() {
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

  void setFocus(float f) {
    focus_ = f;
  }

  void setLens(float l) {
    lens_ = l;
  }

private:
  RGBAColor illuminate(const IntersectionInfo& info, int giDepth, UniformRNGInfo &rngInfo);
  RGBAColor raytrace(const Vector3D& origin, const Vector3D& direction, int depth, int giDepth, UniformRNGInfo &rngInfo);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);
  void expose(PNG *img);
  /**
   * threadTaskDefault - default worker function for threads.
  */
  void threadTaskDefault(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter);
  /**
   * threadTaskFisheye - fisheye render worker function for threads.
  */
  void threadTaskFisheye(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter);
  void threadTaskDOF(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter);
  PNG *render(std::function<void (Scene *, PNG *, SafeQueue<RenderTask> *, SafeProgressBar *)> worker, int numThreads);

  std::vector<std::unique_ptr<Object>> objects;
  std::vector<std::unique_ptr<Plane>> planes;
  std::vector<std::unique_ptr<Light>> lights;
  std::vector<std::unique_ptr<Bulb>> bulbs;
  
  int width_;
  int height_;
  std::string filename_;
  Vector3D eye; // POINT; NOT NORMALIZED
  Vector3D forward; // NOT NORMALIZED; LONGER VECTOR FOR NARROWER FIELD OF VIEW
  Vector3D right; // NORMALIZED
  Vector3D up; // NORMALIZED
  float bias_ = 1e-4;
  float exposure = -1.0;
  int maxBounces = 4;
  int numRays = 1;
  std::unique_ptr<BVH> bvh;
  bool fisheye = false;
  int globalIllumination = 0;
  float focus_ = -1.0;
  float lens_ = 0;
};
